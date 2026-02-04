#include "mpesapayment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSqlQuery>
#include <QFile>
#include "databasemanager.h"
#include <paymentmodel.h>

MpesaPayment::MpesaPayment(const MpesaConfig &config, QObject *parent)
    : Payment(parent),
        m_config(config),
        m_pollTimer(new QTimer(this)),
        m_netManager(new QNetworkAccessManager(this)),
        m_retryCount(0)
{


    m_pollTimer->setInterval(5000); /* Poll every 5 seconds */

    // The polling timer triggers verifyStatus() automatically
    connect(m_pollTimer, &QTimer::timeout, this, &MpesaPayment::verifyStatus);
}

MpesaConfig MpesaPayment::loadConfig(const QString &filePath) {
    MpesaConfig config;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open config file:" << filePath << "Using defaults.";
        return config;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    // Navigate to the "mpesa" object in your JSON
    QJsonObject root = doc.object();
    QJsonObject mpesa = root.value("mpesa").toObject();

    if (!mpesa.isEmpty()) {
        config.isSandbox = mpesa.value("isSandbox").toBool();
        config.shortCode = mpesa.value("shortCode").toString();
        config.passKey = mpesa.value("passKey").toString();
        config.consumerKey = mpesa.value("consumerKey").toString();
        config.consumerSecret = mpesa.value("consumerSecret").toString();
        config.isTill = mpesa.value("isTill").toBool();
        config.callbackUrl = mpesa.value("callbackUrl").toString();

        qDebug() << "[M-Pesa] Config loaded successfully. Sandbox:" << config.isSandbox;
    } else {
        qCritical() << "[M-Pesa] JSON structure invalid. 'mpesa' key not found.";
    }

    return config;
}

void MpesaPayment::process(Money amount, const QVariantMap &data) {
    m_amount = amount;
    m_phone = data.value("phone").toString();

    setState(State::Initiated);

    // If phone is provided, we do the full flow.
    // If not, we just fetch/check the token to ensure API is up.
    if (m_phone.isEmpty()) {
        emit messageUpdated("Checking API Status...");
        fetchToken();
    } else {
        emit messageUpdated("Requesting M-Pesa Token...");
        fetchToken(); // In onTokenReceived, you'd then call sendStkPush() only if m_phone is set
    }
}

void MpesaPayment::fetchToken()
{
    QString cached = PaymentModel::instance().getValidToken();

    if (!cached.isEmpty()) {
        qDebug() << "[M-Pesa] Persistent token found. No network call needed.";
        m_accessToken = cached;
        if (!m_phone.isEmpty()) sendStkPush();
        return;
    }

    if(m_config.consumerKey.isEmpty()) {
        qCritical() << "[M-Pesa] Error: Consumer Key is empty. Check config.json";
        return;
    }

    qDebug() << "[M-Pesa] Initiating OAuth Token Fetch...";
    QUrl url("https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials");
    QNetworkRequest req(url);

    QByteArray auth = QString("%1:%2").arg(m_config.consumerKey, m_config.consumerSecret).toUtf8().toBase64();
    req.setRawHeader("Authorization", "Basic " + auth);

    QNetworkReply *reply = m_netManager->get(req);
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onTokenReceived);
}

void MpesaPayment::onTokenReceived() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || reply->error() != QNetworkReply::NoError) {
        qCritical() << "[M-Pesa] Auth Failed:" << (reply ? reply->errorString() : "Unknown");
        emit errorOccurred("M-Pesa Authentication Failed");
        return;
    }

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
    m_accessToken = res.value("access_token").toString();
    int expiresIn = res.value("expires_in").toString().toInt();

    // Calculate expiry (subtract 60s buffer for safety)
    QDateTime expiry = QDateTime::currentDateTime().addSecs(expiresIn - 60);

    qDebug() << "[M-Pesa] New Token Generated. Valid until:" << expiry.toString();

    // In a real app, you'd pass this back to the controller to cache it
    // For now, proceed with the payment flow
    if (!m_phone.isEmpty()) {
        sendStkPush();
    }
}

void MpesaPayment::sendStkPush()
{
    // 1. Data Prep: Convert amount to cents for DB storage
    // int cents = static_cast<int>(m_amount * 100);
    qDebug() << "stk push amount" << m_amount.toKSH();
    m_lastTimestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

    // 2. Log initiation via Singleton
    PaymentModel::instance().insertPayment(
        "ORD-TEMP-123", // Replace with actual salesModel.currentOrderId
        "MPESA_STK",
        m_amount,
        "Admin",        // Replace with currentUser property
        m_checkoutRequestId
        );

    // 3. Network Request to Safaricom
    qDebug() << "[M-Pesa] Sending STK Push to:" << m_phone;

    // QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    m_lastPassword= generatePassword(m_lastTimestamp);

    StkPushRequest request;
    request.BusinessShortCode = m_config.shortCode.toInt();
    request.Password          = m_lastPassword;
    request.Timestamp         = m_lastTimestamp;
    request.TransactionType   = m_config.isTill ? "CustomerBuyGoodsOnline" : "CustomerPayBillOnline";
    // request.Amount            = static_cast<int>(m_amount.toKSH());
    request.Amount            = 1; // Testing with 1 KSH
    request.PartyA            = m_phone;
    request.PartyB            = m_config.shortCode.toInt();
    request.PhoneNumber       = m_phone;
    request.CallBackURL       = "https://example.com/callback";
    request.AccountReference  = m_currentOrderId.isEmpty() ? "Order" : m_currentOrderId;
    request.TransactionDesc   = "POS Payment";

    // QUrl url;
    // if (m_config.isSandbox) {
    //     url = QUrl("https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest");
    // } else {
    //     url = QUrl("https://api.safaricom.co.ke/mpesa/stkpush/v1/processrequest");
    // }
    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest");
    QNetworkRequest req(url);

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) QtPOS/1.0");
    req.setRawHeader("Accept", "application/json");
    req.setTransferTimeout(30000);



    // DEBUG: Ensure the JSON is clean and compact
    // QByteArray jsonData = QJsonDocument(body).toJson(QJsonDocument::Compact);
    // qDebug() << "[M-Pesa] Sending Payload:" << jsonData;

    // QNetworkReply *reply = m_netManager->post(req, jsonData);
    QNetworkReply *reply = m_netManager->post(req, request.toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onStkPushFinished);
    emit messageUpdated("Requesting M-Pesa PIN prompt...");
}

void MpesaPayment::onStkPushFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // 1. Capture and Debug the Raw Response
    QByteArray responseData = reply->readAll();
    qDebug() << "[M-Pesa] STK Push Response Received:" << responseData;

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject res = doc.object();

    // 2. Check for Network or HTTP errors
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "[M-Pesa] Network Error during STK Push:" << reply->errorString();
        setState(State::Failed);
        emit errorOccurred("Network Error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    // 3. Handle Safaricom API Response
    // ResponseCode "0" means the request was accepted for processing
    if (res.value("ResponseCode").toString() == "0") {
        m_checkoutRequestId = res.value("CheckoutRequestID").toString();

        qDebug() << "[M-Pesa] STK Push Successful!";
        qDebug() << " -> MerchantRequestID:" << res.value("MerchantRequestID").toString();
        qDebug() << " -> CheckoutRequestID:" << m_checkoutRequestId;

        // Use the Singleton to update the database with the real CheckoutID
        // This is crucial for the background polling to find the record later
        PaymentModel::instance().updatePaymentStatus(m_checkoutRequestId, "Awaiting PIN");

        setState(State::AwaitingAction);
        emit messageUpdated("Please enter PIN on your phone.");

        m_retryCount = 0;
        m_pollTimer->start(); // Begin background polling
    } else {
        QString errorMsg = res.value("errorMessage").toString();
        if (errorMsg.isEmpty()) errorMsg = "Unknown API Error";

        qWarning() << "[M-Pesa] STK Push Rejected by API:" << errorMsg;

        setState(State::Failed);
        emit errorOccurred("STK Push Failed: " + errorMsg);
    }

    reply->deleteLater();
}

void MpesaPayment::verifyStatus()
{
    emit messageUpdated("Verifying payment...");
    // Polling Logic: Ensure we have both the request ID and the original timestamp
    if (m_checkoutRequestId.isEmpty() || m_lastTimestamp.isEmpty()) return;

    if (m_retryCount++ > 60) { // 60 seconds timeout
        m_pollTimer->stop();
        setState(State::Failed);
        emit errorOccurred("Transaction Timed Out (No PIN entered).");
        return;
    }

    setState(State::Verifying);

    // Safaricom requirement: The password for the query must be generated using
    // the SAME timestamp used in the processrequest call.
    QString password = generatePassword(m_lastTimestamp);

    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode.toInt();
    body["Password"] = password;
    body["Timestamp"] = m_lastTimestamp; // Use original timestamp
    body["CheckoutRequestID"] = m_checkoutRequestId;

    // QString baseUrl = m_config.isSandbox ? "https://sandbox.safaricom.co.ke" : "https://api.safaricom.co.ke";
    // Using the M-Pesa Express Query endpoint
    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpushquery/v1/query");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onQueryFinished);

}

void MpesaPayment::onQueryFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QByteArray data = reply->readAll();
    QJsonObject res = QJsonDocument::fromJson(data).object();

    qDebug() << "[M-Pesa] Query Result:" << data;

    // 1. Handle definitive ResultCode from M-Pesa Express Query
    if (res.contains("ResultCode")) {
        // Convert to string regardless of if it's stored as int or string in JSON
        QString resultCode = res.value("ResultCode").toVariant().toString();
        QString resultDesc = res.value("ResultDesc").toString();

        if (resultCode == "0") {
            // SUCCESS
            m_pollTimer->stop();
            setState(State::Success); // State 3 in QML
            m_currentMessage = "Payment Successful!";
            emit messageUpdated(m_currentMessage);
            emit completed(m_checkoutRequestId);
            PaymentModel::instance().updatePaymentStatus(m_checkoutRequestId, "Success");
        }
        else if (resultCode == "1032") {
            m_currentMessage = "Transaction cancelled by user.";
            m_pollTimer->stop();
            setState(State::Failed); // State 4 in QML
            emit messageUpdated(m_currentMessage);
            PaymentModel::instance().updatePaymentStatus(m_checkoutRequestId, "Cancelled");
        }
        else if (resultCode == "1") {
            // INSUFFICIENT FUNDS
            m_currentMessage = "Error: Insufficient funds in M-Pesa account.";
            m_pollTimer->stop();
            setState(State::Failed);
            emit messageUpdated(m_currentMessage);
            PaymentModel::instance().updatePaymentStatus(m_checkoutRequestId, "Insufficient Funds");
        }
        else if (resultCode == "1037") {
            // TIMEOUT on Safaricom's end - stay in Verifying state and let poll continue
            emit messageUpdated("Still waiting for PIN entry...");
        }
        else {
            // OTHER ERRORS (System error, etc.)
            m_pollTimer->stop();
            setState(State::Failed);
            emit messageUpdated("M-Pesa Error: " + resultDesc);
            PaymentModel::instance().updatePaymentStatus(m_checkoutRequestId, "Failed: " + resultCode);
        }
    }
    // 2. Handle API Errors (e.g., 500 or 404 from the gateway)
    // Inside MpesaPayment::onQueryFinished
    if (res.contains("errorCode")) {
        QString errorCode = res.value("errorCode").toString();

        if (errorCode == "404.001.03") { // Invalid Access Token
            qWarning() << "[M-Pesa] Access Token rejected during Query. Clearing cache...";

            // 1. Wipe the cached token so the next attempt MUST fetch a new one
            QSqlQuery query(DatabaseManager::instance().database());
            query.exec("DELETE FROM oauth_tokens WHERE provider = 'mpesa'");

            // 2. Update the UI
            m_currentMessage = "Connection lost. Re-authenticating...";
            emit messageUpdated(m_currentMessage);

            // 3. Optional: Immediately try to fetch a new token
            fetchToken();
            return;
        }
    }

    reply->deleteLater();
}

void MpesaPayment::cancel()
{
    m_pollTimer->stop();
    setState(State::Cancelled);
    emit messageUpdated("Payment cancelled.");
}

QString MpesaPayment::generatePassword(const QString &timestamp)
{
    qDebug() << "generate password shortcode" << m_config.shortCode;
    QString raw = m_config.shortCode + m_config.passKey + timestamp;
    return QString(raw.toUtf8().toBase64());
}

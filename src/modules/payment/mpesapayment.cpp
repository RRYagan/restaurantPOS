#include "mpesapayment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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


    m_pollTimer->setInterval(10000); /* Poll every 5 seconds */

    // The polling timer triggers verifyStatus() automatically
    connect(m_pollTimer, &QTimer::timeout, this, &MpesaPayment::verifyStatus);
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
    // PaymentModel::instance().insertPayment(
    //     "ORD-TEMP-123", // Replace with actual salesModel.currentOrderId
    //     "MPESA_STK",
    //     m_amount,
    //     "Admin",        // Replace with currentUser property
    //     m_checkoutRequestId
    //     );

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

void MpesaPayment::onStkPushFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
    m_lastResponse.resultCode = res.value("ResponseCode").toString();
    m_checkoutRequestId = res.value("CheckoutRequestID").toString();

    if (reply->error() == QNetworkReply::NoError && m_lastResponse.resultCode == "0") {
        // NOTIFY CONTROLLER TO CREATE DB RECORD
        emit stkPushInitiated(m_checkoutRequestId, res.value("MerchantRequestID").toString());

        updateStatus(State::AwaitingAction, "Please enter PIN on your phone.");
        m_retryCount = 0;
        m_pollTimer->start();
    } else {
        updateStatus(State::Failed, res.value("ResponseDescription").toString());
    }
    reply->deleteLater();
}

void MpesaPayment::onQueryFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
    QString resultCode = res.value("ResultCode").toString();

    if (resultCode == "0") {
        m_pollTimer->stop();
        QString receipt = res.value("MpesaReceiptNumber").toString();
        // NOTIFY SUCCESS
        emit paymentCompleted(m_checkoutRequestId, receipt);
        emit paymentStatusUpdated(m_checkoutRequestId, "Completed", receipt);
        updateStatus(State::Success, "Payment Successful!");
    } else if (resultCode == "1032") {
        m_pollTimer->stop();
        // NOTIFY CANCEL
        emit paymentCancelled(m_checkoutRequestId);
        // emit paymentStatusUpdated(m_checkoutRequestId, "Cancelled", receipt);
        updateStatus(State::Cancelled, "Transaction Cancelled.");
    } else if (!resultCode.isEmpty()) {
        m_pollTimer->stop();
        // NOTIFY FAILURE
        emit paymentFailed(m_checkoutRequestId, res.value("ResultDesc").toString());
        // emit paymentStatusUpdated(m_checkoutRequestId, "Failed", receipt);
        updateStatus(State::Failed, "Transaction Failed.");
    }
    reply->deleteLater();
}
// Helper to keep logic dry
void MpesaPayment::updateStatus(State state, const QString &msg) {
    m_state = state;
    m_currentMessage = msg;
    emit stateChanged();
    emit messageUpdated(m_currentMessage);
}
void MpesaPayment::verifyStatus()
{

    if (m_checkoutRequestId.isEmpty()) return;

    // Increase timeout: Safaricom allows up to 60s for PIN entry.
    // If polling every 3s, retry limit should be ~20-25.
    if (m_retryCount++ > 25) {
        m_pollTimer->stop();
        updateStatus(State::Failed, "Timed out. Please try again.");
        return;
    }

    // Prepare Request (Standard Query Logic)
    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = generatePassword(m_lastTimestamp);
    body["Timestamp"] = m_lastTimestamp;
    body["CheckoutRequestID"] = m_checkoutRequestId;

    // QString baseUrl = m_config.isSandbox ? "https://sandbox.safaricom.co.ke" : "https://api.safaricom.co.ke";
    // Using the M-Pesa Express Query endpoint
    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpushquery/v1/query");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onQueryFinished);
emit messageUpdated("Verifying payment...");
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

void MpesaPayment::handleTokenTimeout()
{
    qWarning() << "[M-Pesa] Access Token rejected. Cleaning cache...";

    // Clear the expired token from the DB
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM oauth_tokens WHERE provider = :prov");
    query.bindValue(":prov", "mpesa");
    query.exec();

    // Update the UI via our helper
    updateStatus(State::Verifying, "Connection lost. Re-authenticating...");

    // Fetch a fresh token
    fetchToken();
}

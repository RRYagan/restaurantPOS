#include "mpesapayment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSqlQuery>
#include <databasemanager.h>
#include <paymentmodel.h>

MpesaPayment::MpesaPayment(const MpesaConfig &config, QObject *parent)
    : Payment(parent), m_config(config), m_retryCount(0)
{
    m_netManager = new QNetworkAccessManager(this);
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(5000); // Poll every 5 seconds

    // The polling timer triggers verifyStatus() automatically
    connect(m_pollTimer, &QTimer::timeout, this, &MpesaPayment::verifyStatus);
}

void MpesaPayment::process(double amount, const QVariantMap &data) {
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
    int cents = static_cast<int>(m_amount * 100);

    // 2. Log initiation via Singleton
    PaymentModel::instance().insertPayment(
        "ORD-TEMP-123", // Replace with actual salesModel.currentOrderId
        "MPESA_STK",
        cents,
        "Admin",        // Replace with currentUser property
        m_checkoutRequestId
        );

    // 3. Network Request to Safaricom
    qDebug() << "[M-Pesa] Sending STK Push to:" << m_phone;

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString password = generatePassword(timestamp);

    QJsonObject body;
    body["BusinessShortCode"] = 174379;
    qDebug() <<"Business Short code" << m_config.shortCode;
    body["Password"] = password;
    body["Timestamp"] = timestamp;
    body["TransactionType"] = m_config.isTill ? "CustomerBuyGoodsOnline" : "CustomerPayBillOnline";
    body["Amount"] = 1;
    body["PartyA"] = "254728417478";
    body["PartyB"] = 174379;
    body["PhoneNumber"] = "254728417478";
    body["CallBackURL"] = "https://example.com/callback"; // Required but unused in desktop polling
    body["AccountReference"] = "OrderREF";
    body["TransactionDesc"] = "Payment";

    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest");
    QNetworkRequest req(url);

    // ESSENTIAL: Set these headers to bypass the basic WAF filters
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) QtPOS/1.0");
    req.setRawHeader("Accept", "application/json");

    // DEBUG: Ensure the JSON is clean and compact
    QByteArray jsonData = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qDebug() << "[M-Pesa] Sending Payload:" << jsonData;

    QNetworkReply *reply = m_netManager->post(req, jsonData);
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onStkPushFinished);
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
    // Polling Logic
    if (m_checkoutRequestId.isEmpty()) return;

    // Check timeout (e.g. stop after 60 seconds)
    if (m_retryCount++ > 12) {
        m_pollTimer->stop();
        setState(State::Failed);
        emit errorOccurred("Transaction Timed Out (No PIN entered).");
        return;
    }

    setState(State::Verifying);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    // QString password = generatePassword(timestamp);
    QString rawPassword = m_config.shortCode + m_config.passKey + timestamp;
    QByteArray password = rawPassword.toUtf8().toBase64();

    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = QString(password);
    body["Timestamp"] = timestamp;
    body["CheckoutRequestID"] = m_checkoutRequestId;

    QNetworkRequest req(QUrl("https://sandbox.safaricom.co.ke/mpesa/stkpushquery/v1/query"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onQueryFinished);
}

void MpesaPayment::onQueryFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();

    if (res.contains("ResultCode")) {
        QString resultCode = res.value("ResultCode").toString();

        if (resultCode == "0") {
            // Success!
            m_pollTimer->stop();
            setState(State::Success);
            emit completed(m_checkoutRequestId);
            emit messageUpdated("Payment Successful!");
        }
        else if (resultCode == "1032") {
            // User Cancelled
            m_pollTimer->stop();
            setState(State::Cancelled);
            emit errorOccurred("User Cancelled the transaction.");
        }
        else if (resultCode != "1037" && resultCode != "1") {
            // Any other definitive error (Not a timeout or generic error)
            m_pollTimer->stop();
            setState(State::Failed);
            emit errorOccurred(res.value("ResultDesc").toString());
        }
    }
    else if (res.contains("errorCode")) {
        // If API returns errorCode 503.001.something, it often means "still processing"
        // We do nothing here, just let the timer tick again.
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
    QString raw = "174379" + m_config.passKey + timestamp;
    return QString(raw.toUtf8().toBase64());
}

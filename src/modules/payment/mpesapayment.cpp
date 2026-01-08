#include "mpesapayment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

MpesaPayment::MpesaPayment(const MpesaConfig &config, QObject *parent)
    : Payment(parent), m_config(config), m_retryCount(0)
{
    m_netManager = new QNetworkAccessManager(this);
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(5000); // Poll every 5 seconds

    // The polling timer triggers verifyStatus() automatically
    connect(m_pollTimer, &QTimer::timeout, this, &MpesaPayment::verifyStatus);
}

void MpesaPayment::process(double amount, const QVariantMap &data)
{
    m_amount = amount;
    m_phone = data.value("phone").toString();

    if (m_phone.isEmpty()) {
        emit errorOccurred("Phone number is required.");
        setState(State::Failed);
        return;
    }

    setState(State::Initiated);
    emit messageUpdated("Requesting M-Pesa Token...");

    // Step 1: Get OAuth Token
    fetchToken();
}

void MpesaPayment::fetchToken()
{
    QUrl url("https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials");
    QNetworkRequest req(url);

    QByteArray auth = QString("%1:%2").arg(m_config.consumerKey, m_config.consumerSecret).toUtf8().toBase64();
    req.setRawHeader("Authorization", "Basic " + auth);

    QNetworkReply *reply = m_netManager->get(req);
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onTokenReceived);
}

void MpesaPayment::onTokenReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        m_accessToken = json.value("access_token").toString();

        emit messageUpdated("Sending STK Push to phone...");
        sendStkPush(); // Step 2: Send STK Push
    } else {
        setState(State::Failed);
        emit errorOccurred("M-Pesa Auth Failed: " + reply->errorString());
    }
    reply->deleteLater();
}

void MpesaPayment::sendStkPush()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString password = generatePassword(timestamp);

    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = password;
    body["Timestamp"] = timestamp;
    body["TransactionType"] = m_config.isTill ? "CustomerBuyGoodsOnline" : "CustomerPayBillOnline";
    body["Amount"] = static_cast<int>(m_amount);
    body["PartyA"] = m_phone;
    body["PartyB"] = m_config.shortCode;
    body["PhoneNumber"] = m_phone;
    body["CallBackURL"] = "https://example.com/callback"; // Required but unused in desktop polling
    body["AccountReference"] = "OrderREF";
    body["TransactionDesc"] = "Payment";

    QNetworkRequest req(QUrl("https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onStkPushFinished);
}

void MpesaPayment::onStkPushFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();

    // Check ResponseCode. "0" means the push was successfully queued.
    if (res.value("ResponseCode").toString() == "0") {
        m_checkoutRequestId = res.value("CheckoutRequestID").toString();

        setState(State::AwaitingAction);
        emit messageUpdated("Please enter PIN on your phone.");

        m_retryCount = 0;
        m_pollTimer->start(); // Step 3: Start polling
    } else {
        setState(State::Failed);
        emit errorOccurred("STK Push Failed: " + res.value("errorMessage").toString());
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
    QString password = generatePassword(timestamp);

    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = password;
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
    QString raw = m_config.shortCode + m_config.passKey + timestamp;
    return raw.toUtf8().toBase64();
}

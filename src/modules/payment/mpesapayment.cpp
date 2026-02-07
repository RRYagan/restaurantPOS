#include "mpesapayment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSqlQuery>
#include <QFile>
#include "databasemanager.h"
#include <paymentmodel.h>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
const int MAX_POLL_RETRIES = 12;

MpesaPayment::MpesaPayment(const MpesaConfig &config, QObject *parent)
    : Payment(parent),
        m_config(config),
        m_pollTimer(new QTimer(this)),
        m_netManager(new QNetworkAccessManager(this)),
        m_retryCount(0)

{
    m_isRetrying = false;

    m_pollTimer->setInterval(2000); /* Poll every 5 seconds */

    // The polling timer triggers verifyStatus() automatically
connect(m_pollTimer, &QTimer::timeout, this, &MpesaPayment::onPollTimerTimeout);
}


void MpesaPayment::process(Money amount, const QVariantMap &data) {
    m_amount = amount;
    m_phone = data.value("phone").toString();

    setState(PaymentStatus::Initiated);

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
        m_accessToken = cached;
        // Check if we actually have a phone number to process
        if (!m_phone.isEmpty()) {
            sendStkPush();
        } else {
            qDebug() << "[M-Pesa] Token cached and ready for next transaction.";
        }
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

    // M-Pesa often returns expires_in as a string "3599"
    int expiresIn = res.value("expires_in").toString().toInt();
    QDateTime expiry = QDateTime::currentDateTime().addSecs(expiresIn - 60);

    // --- Database Persistence ---
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO oauth_tokens (provider, access_token, expiry_time) "
                  "VALUES (:provider, :token, :expiry)");

    query.bindValue(":provider", "mpesa");
    query.bindValue(":token", m_accessToken);
    // Convert to ISO format string for SQLite DATETIME compatibility
    query.bindValue(":expiry", expiry.toString(Qt::ISODate));

    if (!query.exec()) {
        qCritical() << "[M-Pesa] DB Save Error:" << query.lastError().text();
    } else {
        qDebug() << "[M-Pesa] Token cached in database. Expiry:" << expiry.toString();
    }
    // ----------------------------

    if (!m_phone.isEmpty()) {
        sendStkPush();
    }

    reply->deleteLater();
}

void MpesaPayment::sendStkPush()
{
    if (m_config.shortCode.isEmpty() || m_accessToken.isEmpty()) {
        updateStatus(PaymentStatus::Failed, "Configuration error (Shortcode/Token missing)");
        return;
    }
    qDebug() << "stk push amount" << m_amount.toKSH();
    m_lastTimestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

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

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray rawData = reply->readAll();
    QJsonObject res = QJsonDocument::fromJson(rawData).object();

    qDebug() << "[M-Pesa] STK Response Recv - HTTP:" << statusCode;

    // --- 401 Unauthorized Retry Logic ---
    if (statusCode == 401 && !m_isRetrying) {
        qWarning() << "[M-Pesa] Token expired at runtime. Refreshing...";
        m_isRetrying = true;
        PaymentModel::instance().clearToken("mpesa");
        this->fetchToken();
        reply->deleteLater();
        return;
    }

    m_lastResponse.resultCode = res.value("ResponseCode").toString();
    m_checkoutRequestId = res.value("CheckoutRequestID").toString();

    if (reply->error() == QNetworkReply::NoError && m_lastResponse.resultCode == "0") {
        qDebug() << "[M-Pesa] STK Push Accepted. CheckoutID:" << m_checkoutRequestId;
        m_isRetrying = false; // Reset on success

        emit stkPushInitiated(m_checkoutRequestId, res.value("MerchantRequestID").toString());
        updateStatus(PaymentStatus::AwaitingAction, "Please enter PIN on your phone.");

        QTimer::singleShot(5000, this, [this]() {
            if (m_state == PaymentStatus::AwaitingAction) { // Only start if not cancelled
                m_pollTimer->start(3000); // Poll every 3 seconds
                qDebug() << "[M-Pesa] Initial grace period over. Polling started.";
            }
        });
    } else {
        qCritical() << "[M-Pesa] STK Push Error:" << res.value("ResponseDescription").toString();
        updateStatus(PaymentStatus::Failed, res.value("ResponseDescription").toString());
    }
    reply->deleteLater();
}

void MpesaPayment::onQueryFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
    QString resultCode = res.value("ResultCode").toString();

    qDebug() << "[M-Pesa] Polling Status for ID:" << m_checkoutRequestId << "ResultCode:" << resultCode;
    // Safaricom ResultCodes:
    // "0" -> Success
    // "1032" -> User Cancelled
    // "1037" -> Timeout (Safaricom side)
    // "500xx" -> Request in progress / Pending
    if (resultCode == "0") {
        m_pollTimer->stop();
        QString receipt = res.value("MpesaReceiptNumber").toString();
        qDebug() << "[M-Pesa] Payment Confirmed! Receipt:" << receipt;

        emit paymentCompleted(m_checkoutRequestId, receipt);
        emit paymentStatusUpdated(m_checkoutRequestId, "Completed", receipt);
        updateStatus(PaymentStatus::Success, "Payment Successful!");

    } else if (resultCode == "1032") {
        m_pollTimer->stop();
        qWarning() << "[M-Pesa] User cancelled the USSD prompt.";

        emit paymentCancelled(m_checkoutRequestId);
        emit paymentStatusUpdated(m_checkoutRequestId, "Cancelled", "");
        updateStatus(PaymentStatus::Cancelled, "Transaction Cancelled .");

    } else if (!resultCode.isEmpty()) {
        m_pollTimer->stop();
        QString desc = res.value("ResultDesc").toString();
        qCritical() << "[M-Pesa] Payment Failed. Code:" << resultCode << "Desc:" << desc;

        emit paymentFailed(m_checkoutRequestId, desc);
        emit paymentStatusUpdated(m_checkoutRequestId, "Failed", "");
        updateStatus(PaymentStatus::Failed, "Transaction Failed: " + desc);
    }

    reply->deleteLater();
}

void MpesaPayment::updateStatus(PaymentStatus::State state, const QString &msg) {
    qDebug() << "[Mpesa] UI Transition ->" << state << ":" << msg;

    // Use the setter so the base class 'state()' property is updated
    this->setState(state);

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
        updateStatus(PaymentStatus::Failed, "Timed out. Please try again.");
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


void MpesaPayment::onPollTimerTimeout() {
    m_retryCount++;

    // Instead of calling a different function, run the query directly
    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = generatePassword(m_lastTimestamp);
    body["Timestamp"] = m_lastTimestamp;
    body["CheckoutRequestID"] = m_checkoutRequestId;

    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpushquery/v1/query");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (!reply) return;

        QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
        QString resultCode = res.value("ResultCode").toString();

        // "0" = Success, "1032" = Cancelled
        if (resultCode == "0") {
            m_pollTimer->stop();
            updateStatus(PaymentStatus::Success, "Payment Successful!");
            emit paymentStatusUpdated(m_checkoutRequestId, "Completed", res.value("MpesaReceiptNumber").toString());
        }
        else if (resultCode == "1032") {
            m_pollTimer->stop();
            updateStatus(PaymentStatus::Cancelled, "User cancelled.");
            emit paymentStatusUpdated(m_checkoutRequestId, "Cancelled", "");
        }
        else if (m_retryCount >= MAX_POLL_RETRIES) {
            m_pollTimer->stop();
            updateStatus(PaymentStatus::Failed, "Polling timed out.");
            emit paymentStatusUpdated(m_checkoutRequestId, "Timed Out", "");
        }
        // If resultCode is empty or something else, we let the timer tick again
        reply->deleteLater();
    });
}
void MpesaPayment::queryTransactionStatus() {
    qDebug() << "[M-Pesa] Sending Status Query Request...";

    QUrl url("https://sandbox.safaricom.co.ke/mpesa/stkpushquery/v1/query");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    // Build the query body (BusinessShortCode, Password, Timestamp, CheckoutRequestID)
    QJsonObject body;
    body["BusinessShortCode"] = m_config.shortCode;
    body["Password"] = generatePassword(m_lastTimestamp);
    body["Timestamp"] = m_lastTimestamp;
    body["CheckoutRequestID"] = m_checkoutRequestId;

    QNetworkReply *reply = m_netManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &MpesaPayment::onQueryFinished);
}

void MpesaPayment::cancel()
{
    m_pollTimer->stop();
    setState(PaymentStatus::Cancelled);
    emit messageUpdated("Payment cancelled.");
    updateStatus(PaymentStatus::Cancelled, "Transaction Cancelled.");
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
    updateStatus(PaymentStatus::Verifying, "Connection lost. Re-authenticating...");

    // Fetch a fresh token
    fetchToken();
}

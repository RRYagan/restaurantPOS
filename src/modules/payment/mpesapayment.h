#ifndef MPESAPAYMENT_H
#define MPESAPAYMENT_H

#include "payment.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <money.h>

struct MpesaConfig {
    QString consumerKey;
    QString consumerSecret;
    QString shortCode;
    QString passKey;
    QString callbackUrl; // Added: required for the payload
    bool isSandbox;
    bool isTill;
};

struct StkPushRequest {
    bool isSandbox;
    int BusinessShortCode;
    QString Password;
    QString Timestamp;
    QString TransactionType;
    int Amount;
    QString PartyA;
    int PartyB;
    QString PhoneNumber="254728417478";
    QString CallBackURL;
    QString AccountReference;
    QString TransactionDesc;

    QByteArray toJson() const {
        QVariantMap map;
        map["BusinessShortCode"] = BusinessShortCode;
        map["Password"] = Password;
        map["Timestamp"] = Timestamp;
        map["TransactionType"] = TransactionType;
        map["Amount"] = Amount;
        map["PartyA"] = PartyA;
        map["PartyB"] = PartyB;
        map["PhoneNumber"] = PhoneNumber;
        map["CallBackURL"] = CallBackURL;
        map["AccountReference"] = AccountReference;
        map["TransactionDesc"] = TransactionDesc;
        return QJsonDocument::fromVariant(map).toJson(QJsonDocument::Compact);
    }
};

struct StkPushResponse {
    QString MerchantRequestID;
    QString CheckoutRequestID;
    QString ResponseCode;
    QString ResponseDescription;
    QString CustomerMessage;

    static StkPushResponse fromJson(const QByteArray &data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        StkPushResponse res;
        res.MerchantRequestID   = obj["MerchantRequestID"].toString();
        res.CheckoutRequestID   = obj["CheckoutRequestID"].toString();
        res.ResponseCode        = obj["ResponseCode"].toString();
        res.ResponseDescription = obj["ResponseDescription"].toString();
        res.CustomerMessage     = obj["CustomerMessage"].toString();
        return res;
    }
};

struct MpesaResponse {
    QString merchantRequestId;
    QString checkoutRequestId;
    QString resultCode;
    QString resultDesc;
    QString mpesaReceipt;

    // This is the member function the compiler says is missing
    void reset() {
        merchantRequestId = "";
        checkoutRequestId = "";
        resultCode = "";
        resultDesc = "";
        mpesaReceipt = "";
    }

    QVariantMap toMap() const {
        return QVariantMap {
            {"merchantId", merchantRequestId},
            {"checkoutId", checkoutRequestId},
            {"resultCode", resultCode},
            {"description", resultDesc},
            {"receipt", mpesaReceipt}
        };
    }
};


class MpesaPayment : public Payment {
    Q_OBJECT

    Q_PROPERTY(QVariantMap lastResponse READ lastResponse NOTIFY responseChanged)
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY messageUpdated)
    Q_PROPERTY(int currentState READ currentState NOTIFY stateChanged)
public:
    // Pass the config in the constructor
    explicit MpesaPayment(const MpesaConfig &config, QObject *parent = nullptr);

    QVariantMap lastResponse() const { return m_lastResponse.toMap(); }
    QString currentMessage() const { return m_currentMessage; }
    int currentState() const { return static_cast<int>(m_state); }

    void process(Money amount, const QVariantMap &data) override;
    void cancel() override;
    void verifyStatus() override;
    void preparePayment()
    {
        m_retryCount = 0;
        m_checkoutRequestId = "";
        m_lastTimestamp = "";
        m_lastResponse.reset(); // Clear the struct data

        // Set state to 0 (Idle) so the BusyIndicator doesn't spin immediately
        setState(State::Initiated);

        m_currentMessage = "Ready to send STK Push...";
        emit messageUpdated(m_currentMessage);
        emit responseChanged();
    }

private slots:
    void onTokenReceived();
    // void onStkPushFinished(QNetworkReply *reply);
    void onStkPushFinished();
    void onQueryFinished();

signals:
    void messageUpdated(const QString &message);
    void stateChanged();
    void responseChanged();
    void stkPushInitiated(QString checkoutRequestId, QString merchantRequestId);
    void paymentStatusUpdated(QString checkoutRequestId, QString status, QString receipt = "");
    void paymentCompleted(const QString &checkoutId, const QString &receipt);
    void paymentFailed(QString checkoutId, QString reason);
    void paymentCancelled(QString checkoutId);
private:
    void fetchToken();
    void sendStkPush();
    QString generatePassword(const QString &timestamp);
    void handleTokenTimeout();
    void updateStatus(State state, const QString &msg);

    MpesaConfig m_config;
    MpesaResponse m_lastResponse;

    QNetworkAccessManager *m_netManager;
    QTimer *m_pollTimer;
    QString m_currentOrderId;
    QString m_accessToken;
    QString m_checkoutRequestId;
    Money m_amount{0};
    QString m_phone;
    int m_retryCount;
    QString m_lastTimestamp;
    QString m_lastPassword;
    QString m_currentMessage;
};

#endif // MPESAPAYMENT_H

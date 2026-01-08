#ifndef MPESAPAYMENT_H
#define MPESAPAYMENT_H

#include "payment.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

struct MpesaConfig {
    QString consumerKey;
    QString consumerSecret;
    QString shortCode;      // Paybill or Till Number
    QString passKey;        // Lipa Na Mpesa Online Passkey
    bool isTill;            // True if Buy Goods, False if Paybill
};

class MpesaPayment : public Payment {
    Q_OBJECT
public:
    // Pass the config in the constructor
    explicit MpesaPayment(const MpesaConfig &config, QObject *parent = nullptr);

    void process(double amount, const QVariantMap &data) override;
    void cancel() override;
    void verifyStatus() override;

private slots:
    void onTokenReceived();
    void onStkPushFinished();
    void onQueryFinished();

private:
    void fetchToken();
    void sendStkPush();
    QString generatePassword(const QString &timestamp);

    MpesaConfig m_config;
    QNetworkAccessManager *m_netManager;
    QTimer *m_pollTimer;

    QString m_accessToken;
    QString m_checkoutRequestId;
    double m_amount;
    QString m_phone;
    int m_retryCount;
};

#endif // MPESAPAYMENT_H

#ifndef PAYMENTMODEL_H
#define PAYMENTMODEL_H

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include "databasemanager.h"
#include "money.h"

class PaymentModel : public QObject {
    Q_OBJECT
public:
    explicit PaymentModel(QObject *parent = nullptr);

    static PaymentModel& instance() {
        static PaymentModel _instance;
        return _instance;
    }
    // Delete copy constructors
    PaymentModel(const PaymentModel&) = delete;
    void operator=(const PaymentModel&) = delete;

    void cacheToken(const QString &token, int expiresInSeconds);

    QString getValidToken();
    void saveMpesaToken(const QString &token, int expiresIn);

    /**
     * @brief Inserts a new payment record into the database
     * @param orderId Internal UUID or Order ID
     * @param type e.g., "MPESA_STK", "CASH", "MPESA_QR"
     * @param amount Amount in cents (to match your schema)
     * @param userTag The person who initiated the payment
     * @param externalRef M-Pesa CheckoutRequestID or Receipt
     */
    bool insertPayment(const QString &orderId,
                       const QString &type,
                       Money amountCents,
                       const QString &userTag,
                       const QString &externalRef = "");

    /**
     * @brief Update the status of an existing payment
     */
    bool updatePaymentStatus(const QString &externalRef, const QString &newStatus);
    bool updateTransactionId(const QString &orderId, const QString &transactionId);
    bool clearAllPayments();

signals:
    void paymentLogged(QString orderId, bool success);
};

#endif // PAYMENTMODEL_H

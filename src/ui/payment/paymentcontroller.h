#ifndef PAYMENTCONTROLLER_H
#define PAYMENTCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>
#include "../../src/modules/payment/payment.h"
#include "../../src/modules/payment/cashpayment.h"
#include "../../src/modules/payment/mpesapayment.h"
#include <QtQml/qqmlregistration.h>

class PaymentController : public QObject {
    Q_OBJECT
    QML_ELEMENT // Makes it available in QML as 'PaymentController'

    // Properties for QML data binding
    Q_PROPERTY(Payment::State currentState READ currentState NOTIFY stateChanged)
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY messageUpdated)
    Q_PROPERTY(double amount READ amount WRITE setAmount NOTIFY amountChanged)

public:
    explicit PaymentController(QObject *parent = nullptr);
    ~PaymentController();

    // --- QML Invokable Methods ---
    Q_INVOKABLE void startCashPayment();
    Q_INVOKABLE void startMpesaPayment(const QString &phone);
    Q_INVOKABLE void confirmAction(); // Used for Cash "Confirm Received"
    Q_INVOKABLE void cancelPayment();

    // Getters
    Payment::State currentState() const;
    QString currentMessage() const;
    double amount() const { return m_amount; }
    void setAmount(double a) { if(m_amount != a) { m_amount = a; emit amountChanged(); }}

signals:
    void stateChanged();
    void messageUpdated();
    void amountChanged();
    void paymentFinished(bool success, QString receiptId);

private:
    void cleanUpActivePayment();
    void connectSignals();

    Payment* m_activePayment = nullptr;
    QString m_message;
    double m_amount = 0.0;

    // M-Pesa config (In a real app, load this from a secure config/DB)
    MpesaConfig m_mpesaConfig;
};

#endif // PAYMENTCONTROLLER_H

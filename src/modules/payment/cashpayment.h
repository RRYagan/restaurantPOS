#ifndef CASHPAYMENT_H
#define CASHPAYMENT_H

#include "payment.h"

class CashPayment : public Payment {
    Q_OBJECT
public:
    explicit CashPayment(QObject *parent = nullptr);

    void process(double amount, const QVariantMap &data) override;
    void cancel() override;
    void verifyStatus() override;

private:
    double m_amount = 0.0;
};

#endif // CASHPAYMENT_H

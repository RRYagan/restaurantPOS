#ifndef CASHPAYMENT_H
#define CASHPAYMENT_H

#include "payment.h"

class CashPayment : public Payment {
    Q_OBJECT
public:
    explicit CashPayment(QObject *parent = nullptr);

    void process(Money amount, const QVariantMap &data) override;
    void cancel() override;
    void verifyStatus() override;
    void preparePayment() override;

private:
    Money m_amount{0};
};

#endif // CASHPAYMENT_H

#include "cashpayment.h"
#include <QDateTime>

CashPayment::CashPayment(QObject *parent) : Payment(parent)
{
}

void CashPayment::process(Money amount, const QVariantMap &data)
{
    Q_UNUSED(data);
    m_amount = amount;

    // 1. Move to AwaitingAction so UI knows we are waiting for human input
    setState(PaymentStatus::AwaitingAction);
    emit messageUpdated(QString("Please collect KES %1 from the customer.").arg(amount.toKSH()));
}

void CashPayment::cancel()
{
    setState(PaymentStatus::Cancelled);
    emit messageUpdated("Cash transaction cancelled.");
}

void CashPayment::verifyStatus() {
    if (m_state == PaymentStatus::AwaitingAction) {
        // Change state first
        setState(PaymentStatus::Success);

        QString receipt = QString("CASH-%1").arg(QDateTime::currentMSecsSinceEpoch());

        // Ensure we emit the receipt so SalesViewController::finalizeTransaction is called
        emit messageUpdated("Cash received successfully.");
        emit completed(receipt);
    } else {
        qWarning() << "CashPayment: verifyStatus called while in state" << m_state;
    }
}

void CashPayment::preparePayment() {
    setState(PaymentStatus::Initiated);
    emit messageUpdated("Ready for cash payment...");
}

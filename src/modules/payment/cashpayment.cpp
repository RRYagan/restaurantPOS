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
    setState(State::AwaitingAction);
    emit messageUpdated(QString("Please collect KES %1 from the customer.").arg(amount.toKSH()));
}

void CashPayment::cancel()
{
    setState(State::Cancelled);
    emit messageUpdated("Cash transaction cancelled.");
}

void CashPayment::verifyStatus()
{
    // This is triggered manually by the UI "Confirm Payment" button
    if (m_state == State::AwaitingAction) {
        setState(State::Success);

        // Generate a local receipt ID
        QString receipt = QString("CASH-%1").arg(QDateTime::currentMSecsSinceEpoch());

        emit messageUpdated("Cash received successfully.");
        emit completed(receipt);
    }
}

void CashPayment::preparePayment() {
    setState(State::Initiated);
    emit messageUpdated("Ready for cash payment...");
}

#ifndef PAYMENT_ENUMS_H
#define PAYMENT_ENUMS_H

#include <QObject>

namespace PaymentStatus {
Q_NAMESPACE

enum State {
    Idle,
    Initiated,
    AwaitingAction,
    Verifying,
    Success,
    Failed,
    Cancelled
};
Q_ENUM_NS(State)
}  // namespace PaymentStatus

#endif

#ifndef PAYMENT_H
#define PAYMENT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <money.h>

class Payment : public QObject {
    Q_OBJECT

public:
    // Universal State Machine
    enum class State {
        Idle,
        Initiated,        // Request sent to provider
        AwaitingAction,   // User needs to act (Enter PIN, Give Cash)
        Verifying,        // Polling status or checking verification
        Success,          // Payment confirmed
        Failed,           // Payment rejected/error
        Cancelled         // User or Teller cancelled
    };
    Q_ENUM(State)

    explicit Payment(QObject *parent = nullptr);
    virtual ~Payment();

    // --- Core Abstract Interface ---

    // Start the payment flow.
    // data: Dynamic params (e.g. {"phone": "2547..."})
    virtual void process(Money amount, const QVariantMap &data = {}) = 0;

    // Stop/Cancel the flow
    virtual void cancel() = 0;

    // Force a status check (Used for Manual Cash or Polling)
    virtual void verifyStatus() = 0;

    // --- Getters ---
    State state() const;
    QString lastError() const;

signals:
    void stateChanged(Payment::State newState);
    void messageUpdated(const QString &msg);
    void completed(const QString &receiptId);
    void errorOccurred(const QString &error);

protected:
    void setState(State s);

    State m_state;
    QString m_lastError;
};

#endif // PAYMENT_H

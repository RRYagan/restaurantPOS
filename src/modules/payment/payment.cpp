#include "payment.h"

Payment::Payment(QObject *parent)
    : QObject(parent), m_state(State::Idle)
{
}

Payment::~Payment()
{
}

Payment::State Payment::state() const
{
    return m_state;
}

QString Payment::lastError() const
{
    return m_lastError;
}

void Payment::setState(State s)
{
    if (m_state != s) {
        m_state = s;
        emit stateChanged(s);
    }
}

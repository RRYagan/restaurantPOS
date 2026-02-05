#include "payment.h"
#include <QMetaEnum>

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

QString Payment::stateToString(int stateValue) const {
    const QMetaObject &mo = Payment::staticMetaObject;
    int index = mo.indexOfEnumerator("State");
    QMetaEnum metaEnum = mo.enumerator(index);

    const char* key = metaEnum.valueToKey(stateValue);
    return key ? QString::fromLatin1(key) : QString("Unknown");
}

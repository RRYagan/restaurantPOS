#ifndef MONEY_H
#define MONEY_H

#pragma once
#include <QMetaType>
#include <QString>

struct Money
{
    Q_GADGET
    Q_PROPERTY(double value READ toDouble CONSTANT)
    Q_PROPERTY(QString formatted READ toString CONSTANT)

public:
    int64_t cents = 0; // stored as 100 for 1.00

    double toDouble() const { return cents / 100.0; }
    QString toString() const { return QString::number(toDouble(), 'f', 2); }

    // operators
    Money operator +(const Money& other) const { return {cents + other.cents};}
    Money operator *(double factor) const {return {static_cast<int64_t>(cents * factor)};}

};
Q_DECLARE_METATYPE(Money)

#endif // MONEY_H

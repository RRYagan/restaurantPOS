#ifndef MONEY_H
#define MONEY_H

#pragma once
#include <QMetaType>
#include <QString>
#include <cmath> // Required for std::round

struct Money
{
    Q_GADGET
    Q_PROPERTY(double value READ toKSH CONSTANT)
    Q_PROPERTY(QString formatted READ toKSHString CONSTANT)

public:
    int64_t cents = 0;

    // Constructors
    Money() = default;
    explicit Money(int64_t c) : cents(c) {}

    // Static Factory: This is how you convert User Input (Double) -> Money
    static Money toCents(double value) {
        // We use std::round to prevent 1.15 becoming 1.14
        return Money(static_cast<int64_t>(std::round(value * 100.0)));
    }

    // Conversion back for UI
    double toKSH() const { return cents / 100.0; }
    QString toKSHString() const { return QString::number(toKSH(), 'f', 2); }

    // Operators
    Money operator +(const Money& other) const { return Money(cents + other.cents); }
    Money operator -(const Money& other) const { return Money(cents - other.cents); }
    bool operator !=(const Money& other) const { return !(*this == other); }

    // Improved multiplication with rounding
    Money operator *(double factor) const {
        return Money(static_cast<int64_t>(std::round(cents * factor)));
    }

    // Comparison (useful for logic like "if (balance < price)")
    bool operator <(const Money& other) const { return cents < other.cents; }
    bool operator ==(const Money& other) const { return cents == other.cents; }
};

Q_DECLARE_METATYPE(Money)

#endif // MONEY_H

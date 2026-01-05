#ifndef ORDER_H
#define ORDER_H

#pragma once
#include "orderitem.h"
#include <QDateTime>

enum class OrderStatus {
    Open,
    SentToKitchen,
    Paid,
    Cancelled
};

struct Order {
    Q_GADGET
public:
    QString orderId;
    int tableNumber;
    QDateTime createdAt;
    OrderStatus status = OrderStatus::Open;
    QList<OrderItem> items;

    Money subtotal() const {
        Money sum{0};
        for (const auto& item: items) sum = sum + item.total();
        return sum;
    }

    Money tax(double rate = 0.08) const {
        return subtotal() * rate;
    }

    Money grandTotal() const {
        return subtotal() + tax();
    }
};

#endif // ORDER_H

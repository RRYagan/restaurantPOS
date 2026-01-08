#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QObject>
#include <QUuid>
#include <QDateTime>
#include "order.h"

class OrderModel : public QObject {
    Q_OBJECT
public:
    explicit OrderModel(QObject *parent = nullptr) : QObject(parent) {}

    bool saveOrder(Order &order);
    Order loadOrder(const QString& orderId);
    int generateOrderId();
};

#endif

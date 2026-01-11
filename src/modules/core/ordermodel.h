#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QObject>
#include <QSqlDatabase>

class OrderModel : public QObject
{
    Q_OBJECT
public:
    explicit OrderModel(QObject* parent = nullptr);

    Q_INVOKABLE bool completeOrder(const QString& orderId);
    Q_INVOKABLE bool applyMenuItemSale(const QString& menuItemId);

private:
    QSqlDatabase db() const;
};

#endif // ORDERMODEL_H


// #ifndef ORDERMODEL_H
// #define ORDERMODEL_H

// #include <QObject>
// #include <QUuid>
// #include <QDateTime>
// #include "order.h"

// class OrderModel : public QObject {
//     Q_OBJECT
// public:
//     explicit OrderModel(QObject *parent = nullptr) : QObject(parent) {}

//     bool saveOrder(Order &order);
//     Order loadOrder(const QString& orderId);
//     int generateOrderId();
// };

// #endif

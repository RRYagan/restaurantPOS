#ifndef ORDERITEM_H
#define ORDERITEM_H

#pragma once
#include <QUuid>
#include "menuitem.h"

struct OrderItem {
    Q_GADGET
public:
    QUuid uniqueId;
    int menuItemId;
    QString name;
    int quantity = 1;
    Money priceAtTimeOfSale;
    QList<Modifier> selectModifiers;

    Money total() const {
        Money t = priceAtTimeOfSale;
        for (const auto& m : selectModifiers) t = t + m.extraPrice;
        return t * quantity;
    }

};

#endif // ORDERITEM_H

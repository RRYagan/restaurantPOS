#ifndef ORDERITEM_H
#define ORDERITEM_H

#pragma once
#include <QUuid>
#include "menuitem.h"

struct OrderItem {
    Q_GADGET
public:
    QString id;
    int menuItemId;
    QString name;
    int quantity = 1;
    Money price;
    QList<Modifier> selectModifiers;

    Money total() const {
        Money t = price;
        for (const auto& m : selectModifiers) t = t + m.extraPrice;
        return t * quantity;
    }

};

#endif // ORDERITEM_H

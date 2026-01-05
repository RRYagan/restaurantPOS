#ifndef INVENTORYITEM_H
#define INVENTORYITEM_H

#include <QString>


struct InventoryItem {
    int id;
    QString name;
    int quantity;
    QString unit;
};

#endif // INVENTORYITEM_H

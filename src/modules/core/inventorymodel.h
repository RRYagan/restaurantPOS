#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QObject>
#include <QVariantList>

class InventoryModel : public QObject {
    Q_OBJECT
public:
    QVariantList fetchInventory();
    bool addInventoryItem(const QString &name, int quantity, const QString &unit);
    bool updateInventoryItem(int id, const QString &name, int quantity, const QString &unit);
    bool deleteInventoryItem(int id);
    QVariantList fetchAllInventoryHistory();
};
#endif

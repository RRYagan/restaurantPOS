#include "inventorymodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QDateTime>

QVariantList InventoryModel::fetchInventory() {
    QVariantList items;
    QSqlQuery query("SELECT id, name, quantity, unit FROM inventory", DatabaseManager::instance().database());
    while (query.next()) {
        QVariantMap item;
        item["id"] = query.value(0).toInt();
        item["name"] = query.value(1).toString();
        item["quantity"] = query.value(2).toInt();
        item["unit"] = query.value(3).toString();
        items.append(item);
    }
    return items;
}

QVariantList InventoryModel::fetchAllInventoryHistory() {
    QVariantList history;
    QSqlQuery query("SELECT timestamp, action, change_details, item_id, old_quantity, new_quantity FROM inventory_history ORDER BY timestamp DESC", DatabaseManager::instance().database());
    while (query.next()) {
        QVariantMap entry;
        entry["timestamp"] = query.value(0).toDateTime().toString("yyyy-MM-dd HH:mm");
        entry["action"] = query.value(1).toString();
        entry["details"] = query.value(2).toString();
        entry["item_id"] = query.value(3).toInt();
        entry["oldQty"] = query.value(4).toInt();
        entry["newQty"] = query.value(5).toInt();
        history.append(entry);
    }
    return history;
}

bool InventoryModel::addInventoryItem(const QString &name, int quantity, const QString &unit) {
    QSqlQuery q;
    q.prepare("INSERT INTO inventory (name, quantity, unit) VALUES (?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(quantity);
    q.addBindValue(unit);
    return q.exec();
}

bool InventoryModel::updateInventoryItem(int id, const QString &name, int quantity, const QString &unit) {
    QSqlQuery q;
    q.prepare("UPDATE inventory SET name = ?, quantity = ?, unit = ? WHERE id = ?");
    q.addBindValue(name);
    q.addBindValue(quantity);
    q.addBindValue(unit);
    q.addBindValue(id);
    return q.exec();
}

bool InventoryModel::deleteInventoryItem(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM inventory WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

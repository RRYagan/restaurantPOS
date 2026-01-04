#include "inventorymodel.h"
#include "databasemanager.h"
#include <QSqlQuery>

InventoryModel::InventoryModel(QObject *parent) : QAbstractListModel(parent) {
    refresh();
}

int InventoryModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_items.size();
}

QVariant InventoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) return QVariant();
    const auto &item = m_items[index.row()];
    switch (role) {
    case NameRole: return item.name;
    case QuantityRole: return item.quantity;
    case UnitRole: return item.unit;
    case IdRole: return item.id;
    }
    return QVariant();
}

QHash<int, QByteArray> InventoryModel::roleNames() const {
    return { {NameRole, "name"}, {QuantityRole, "quantity"}, {UnitRole, "unit"}, {IdRole, "id"} };
}

void InventoryModel::refresh() {
    beginResetModel();
    m_items.clear();
    QSqlQuery query("SELECT id, name, quantity, unit FROM inventory");
    while (query.next()) {
        m_items.append({query.value(0).toInt(), query.value(1).toString(),
                        query.value(2).toInt(), query.value(3).toString()});
    }
    endResetModel();
}

// Call DatabaseManager methods
bool InventoryModel::addStock(const QString &name, int qty, const QString &unit) {
    if (DatabaseManager::instance().addInventoryItem(name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

bool InventoryModel::updateStock(int id, const QString &name, int qty, const QString &unit) {
    if (DatabaseManager::instance().updateInventoryItem(id, name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

#include "inventorymodel.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QUuid>
#include <QDebug>

InventoryModel::InventoryModel(QObject* parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    setTable("inventory");
    setEditStrategy(OnManualSubmit);
    cacheIndices();
    select();
}

void InventoryModel::cacheIndices() {
    m_idCol       = fieldIndex("id");
    m_nameCol     = fieldIndex("name");
    m_pkgAvailCol = fieldIndex("packages_available");
    m_pkgUnitCol  = fieldIndex("packaging_unit_id");
    m_qtyAvailCol = fieldIndex("quantity_available");
    m_qtyUnitCol  = fieldIndex("quantity_unit_id");
    m_createdCol  = fieldIndex("created_at");
    m_updatedCol  = fieldIndex("updated_at");
}

// In inventorymodel.cpp
QVariant InventoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    int row = index.row();

    // Handle Unit Name lookups
    if (role == QuantityUnitNameRole || role == PackagingUnitNameRole) {
        int col = (role == QuantityUnitNameRole) ? m_qtyUnitCol : m_pkgUnitCol;
        int unitId = QSqlTableModel::data(this->index(row, col)).toInt();

        QSqlQuery query;
        // Use the appropriate table based on your schema (quantity_unit or packaging_unit)
        QString table = (role == QuantityUnitNameRole) ? "quantity_unit" : "packaging_unit";
        query.prepare(QString("SELECT name FROM %1 WHERE id = :id").arg(table));
        query.bindValue(":id", unitId);

        if (query.exec() && query.next()) return query.value(0).toString();
        return "units";
    }

    // Standard roles
    switch (role) {
    case IdRole:                return QSqlTableModel::data(this->index(row, m_idCol));
    case NameRole:              return QSqlTableModel::data(this->index(row, m_nameCol));
    case QuantityAvailableRole: return QSqlTableModel::data(this->index(row, m_qtyAvailCol));
    case PackagesAvailableRole: return QSqlTableModel::data(this->index(row, m_pkgAvailCol));
    case PackagingUnitIdRole:   return QSqlTableModel::data(this->index(row, m_pkgUnitCol));
    case QuantityUnitIdRole:    return QSqlTableModel::data(this->index(row, m_qtyUnitCol));
    case CreatedAtRole:         return QSqlTableModel::data(this->index(row, m_createdCol));
    case UpdatedAtRole:         return QSqlTableModel::data(this->index(row, m_updatedCol));
    }
    return QSqlTableModel::data(index, role);
}

QHash<int, QByteArray> InventoryModel::roleNames() const {
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { QuantityAvailableRole, "quantityAvailable" },
        { QuantityUnitNameRole, "quantityUnitName" }, // New
        { PackagesAvailableRole, "packagesAvailable" },
        { PackagingUnitNameRole, "packagingUnitName" }, // New
        { CreatedAtRole, "createdAt" },
        { UpdatedAtRole, "updatedAt" }
    };
}
// --- CREATE ---
bool InventoryModel::createItem(const QVariantMap& data) {
    QSqlQuery query(database());
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // We omit created_at/updated_at to let DB handle DEFAULT values
    query.prepare("INSERT INTO inventory (id, name, packages_available, packaging_unit_id, "
                  "quantity_available, quantity_unit_id) VALUES (:id, :name, :pkg, :pUnit, :qty, :qUnit)");

    query.bindValue(":id", newId);
    query.bindValue(":name", data.value("name").toString());
    query.bindValue(":pkg", data.value("packagesAvailable").toInt());
    query.bindValue(":pUnit", data.value("packagingUnitId").toInt());
    query.bindValue(":qty", data.value("quantityAvailable").toDouble());
    query.bindValue(":qUnit", data.value("quantityUnitId").toInt());

    if (query.exec()) {
        select();
        return true;
    }
    qWarning() << "Create error:" << query.lastError().text();
    return false;
}

// --- UPDATE ---
bool InventoryModel::updateItem(const QVariantMap& data) {
    QSqlQuery query(database());
    // Explicitly set updated_at to CURRENT_TIMESTAMP
    query.prepare("UPDATE inventory SET name=:name, packages_available=:pkg, "
                  "packaging_unit_id=:pUnit, quantity_available=:qty, "
                  "quantity_unit_id=:qUnit WHERE id=:id");

    query.bindValue(":id", data.value("id").toString());
    query.bindValue(":name", data.value("name").toString());
    query.bindValue(":pkg", data.value("packagesAvailable").toInt());
    query.bindValue(":pUnit", data.value("packagingUnitId").toInt());
    query.bindValue(":qty", data.value("quantityAvailable").toDouble());
    query.bindValue(":qUnit", data.value("quantityUnitId").toInt());

    if (query.exec()) {
        select();
        return true;
    }
    return false;
}

// --- DELETE ---
bool InventoryModel::removeItem(const QString& id) {
    QSqlQuery query(database());
    query.prepare("DELETE FROM inventory WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec()) {
        select();
        return true;
    }
    return false;
}

InventoryItem InventoryModel::inventoryAt(int row) const {
    if (row < 0 || row >= rowCount()) return InventoryItem();

    QSqlRecord rec = record(row);
    InventoryItem c; // Ensure this is not QList<InventoryItem>

    c.id                = rec.value("id").toString();
    c.name              = rec.value("name").toString();
    c.packagesAvailable = rec.value("packages_available").toInt();
    c.packagingUnitId   = rec.value("packaging_unit_id").toInt();
    c.quantityAvailable = rec.value("quantity_available").toDouble();
    c.quantityUnitId    = rec.value("quantity_unit_id").toInt();
    c.createdAt         = rec.value("created_at").toString();
    c.updatedAt         = rec.value("updated_at").toString();

    return c;
}

// --- READ ALL ---
QList<InventoryItem> InventoryModel::allItems() {
    QList<InventoryItem> inventory_items;
    for (int i = 0; i < rowCount(); ++i) {
        inventory_items.append(inventoryAt(i));
    }

    return inventory_items;
}

#include "inventorymodel.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QUuid>
#include <QDebug>

InventoryModel::InventoryModel(QObject* parent, const QSqlDatabase& db)
    : QSqlTableModel(parent, db)
{
    setTable("inventory");
    setEditStrategy(OnManualSubmit);
    cacheIndices();
    select();
}

auto InventoryModel::cacheIndices() -> void {
    m_idCol       = fieldIndex("id");
    m_nameCol     = fieldIndex("name");
    m_pkgAvailCol = fieldIndex("total_packages_available");
    m_pkgUnitCol  = fieldIndex("packaging_unit_id");
    m_qpPkgCol    = fieldIndex("quantity_per_package");
    m_qtyAvailCol = fieldIndex("total_quantity_available");
    m_qtyUnitCol  = fieldIndex("quantity_unit_id");
    m_createdCol  = fieldIndex("created_at");
    m_updatedCol  = fieldIndex("updated_at");
}

[[nodiscard]] auto InventoryModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid()) return {};

    int row = index.row();

    switch (role) {
    case IdRole:                return QSqlTableModel::data(this->index(row, m_idCol));
    case NameRole:              return QSqlTableModel::data(this->index(row, m_nameCol));
    case QuantityAvailableRole: return QSqlTableModel::data(this->index(row, m_qtyAvailCol));
    case PackagesAvailableRole: return QSqlTableModel::data(this->index(row, m_pkgAvailCol));
    case PackagingUnitNameRole: return QSqlTableModel::data(this->index(row, m_pkgUnitCol));
    case QuantityPerPackageRole: return QSqlTableModel::data(this->index(row, m_qpPkgCol));
    case QuantityUnitNameRole:  return QSqlTableModel::data(this->index(row, m_qtyUnitCol));
    case CreatedAtRole:         return QSqlTableModel::data(this->index(row, m_createdCol));
    case UpdatedAtRole:         return QSqlTableModel::data(this->index(row, m_updatedCol));

    // Add the default case to satisfy the linter and handle standard Qt roles
    default:
        return QSqlTableModel::data(index, role);
    }
}

[[nodiscard]] auto InventoryModel::roleNames() const -> QHash<int, QByteArray> {
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { QuantityAvailableRole, "quantityAvailable" },
        { QuantityUnitNameRole, "quantityUnitName" },
        { PackagesAvailableRole, "packagesAvailable" },
        { PackagingUnitNameRole, "packagingUnitName" },
        { QuantityPerPackageRole, "quantityPerPackage" },
        { CreatedAtRole, "createdAt" },
        { UpdatedAtRole, "updatedAt" }
    };
}

auto InventoryModel::createItem(const QVariantMap& data) -> bool {
    QSqlQuery query(database());
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare(R"(
        INSERT INTO inventory (
            id, name, total_packages_available, packaging_unit_id,
            quantity_per_package, quantity_unit_id, total_quantity_available
        ) VALUES (
            :id, :name, :pkg, :pUnit, :qp_pkg, :qUnit, :qty
        )
    )");

    query.bindValue(":id", newId);
    query.bindValue(":name", data.value("name").toString());
    query.bindValue(":pkg", data.value("packagesAvailable").toInt());
    query.bindValue(":pUnit", data.value("packagingUnitId").toString());
    query.bindValue(":qp_pkg", data.value("quantityPerPackage").toDouble());
    query.bindValue(":qUnit", data.value("quantityUnitId").toString());
    query.bindValue(":qty", data.value("quantityAvailable").toDouble());

    if (!query.exec()) {
        qCritical() << "==== DATABASE INSERT ERROR ====";
        qCritical() << "Error Text :" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

auto InventoryModel::updateItem(const QVariantMap& data) -> bool {
    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE inventory SET
            name = :name,
            total_packages_available = :pkg,
            packaging_unit_id = :pUnit,
            quantity_per_package = :qp_pkg,
            total_quantity_available = :qty,
            quantity_unit_id = :qUnit
        WHERE id = :id
    )");

    query.bindValue(":id", data.value("id").toString());
    query.bindValue(":name", data.value("name").toString());
    query.bindValue(":pkg", data.value("packagesAvailable").toInt());
    query.bindValue(":pUnit", data.value("packagingUnitId").toString());
    query.bindValue(":qp_pkg", data.value("quantityPerPackage").toDouble());
    query.bindValue(":qty", data.value("quantityAvailable").toDouble());
    query.bindValue(":qUnit", data.value("quantityUnitId").toString());

    if (!query.exec()) {
        qCritical() << "==== DATABASE UPDATE ERROR ====";
        qCritical() << "Error Text :" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

auto InventoryModel::removeItem(const QString& id) -> bool {
    QSqlQuery query(database());
    query.prepare("DELETE FROM inventory WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec()) {
        select();
        return true;
    }
    return false;
}

[[nodiscard]] auto InventoryModel::inventoryAt(int row) const -> InventoryItem {
    if (row < 0 || row >= rowCount()) {
        return {};
    }

    QSqlRecord rec = record(row);
    InventoryItem c;

    c.id                = rec.value(m_idCol).toString();
    c.name              = rec.value(m_nameCol).toString();
    c.packagesAvailable = rec.value(m_pkgAvailCol).toInt();
    c.packagingUnitId   = rec.value(m_pkgUnitCol).toString();
    c.quantityPerPackage = rec.value(m_qpPkgCol).toDouble();
    c.quantityAvailable = rec.value(m_qtyAvailCol).toDouble();
    c.quantityUnitId    = rec.value(m_qtyUnitCol).toString();
    c.createdAt         = rec.value(m_createdCol).toDateTime();
    c.updatedAt         = rec.value(m_updatedCol).toDateTime();

    return c;
}

[[nodiscard]] auto InventoryModel::allItems() -> QList<InventoryItem> {
    QList<InventoryItem> inventory_items;
    inventory_items.reserve(rowCount()); // Pre-allocate memory for performance

    for (int i = 0; i < rowCount(); ++i) {
        inventory_items.append(inventoryAt(i));
    }

    return inventory_items;
}

[[nodiscard]] auto InventoryModel::getItemById(const QString& id) const -> InventoryItem {
    for (int i = 0; i < rowCount(); ++i) {
        // Access the ID column directly from the model data
        QString currentId = index(i, m_idCol).data().toString();

        if (currentId == id) {
            return inventoryAt(i);
        }
    }

    // Return an empty/default InventoryItem if not found
    return {};
}

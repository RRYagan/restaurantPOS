#include "productcompositionmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QUuid>

ProductCompositionModel::ProductCompositionModel(QObject* parent, const QSqlDatabase& db)
    : QSqlTableModel(parent, db)
{
    setTable("product_composition");
    setEditStrategy(OnManualSubmit);

    m_idCol         = fieldIndex("id");
    m_productCol    = fieldIndex("product_id");
    m_ingredientCol = fieldIndex("inventory_id");
    m_qtyCol        = fieldIndex("required_quantity");
    m_unitCol       = fieldIndex("quantity_unit");

    select();
}

[[nodiscard]] auto ProductCompositionModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid()) return {};

    if (role == IngredientNameRole) {
        QString ingId = QSqlTableModel::data(this->index(index.row(), m_ingredientCol)).toString();
        QSqlQuery query(database());
        query.prepare("SELECT name FROM inventory WHERE id = :id");
        query.bindValue(":id", ingId);
        if (query.exec() && query.next()) {
            return query.value(0).toString();
        }
        return "Unknown Item";
    }

    switch (role) {
    case IdRole:           return QSqlTableModel::data(this->index(index.row(), m_idCol));
    case ProductIdRole:    return QSqlTableModel::data(this->index(index.row(), m_productCol));
    case IngredientIdRole: return QSqlTableModel::data(this->index(index.row(), m_ingredientCol));
    case QuantityRole:     return QSqlTableModel::data(this->index(index.row(), m_qtyCol));
    case UnitRole:         return QSqlTableModel::data(this->index(index.row(), m_unitCol));
    default:               return QSqlTableModel::data(index, role);
    }
}

auto ProductCompositionModel::roleNames() const -> QHash<int, QByteArray> {
    return {
        { IdRole, "id" },
        { ProductIdRole, "productId" },
        { IngredientIdRole, "ingredientProductId" },
        { IngredientNameRole, "ingredientName" },
        { QuantityRole, "quantity" },
        { UnitRole, "unitId" }
    };
}

auto ProductCompositionModel::productId() const -> QString {
    return m_productId;
}

auto ProductCompositionModel::setProductId(const QString& id) -> void {
    if (m_productId == id) return;

    m_productId = id;
    setFilter(QString("product_id = '%1'").arg(id));
    select();
    emit productIdChanged();
}

auto ProductCompositionModel::addIngredient(const QVariantMap& data) -> bool {
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO product_composition (
            id, product_id, inventory_id, required_quantity, quantity_unit
        ) VALUES (
            :id, :p_id, :ing_id, :qty, :unit
        )
    )");

    query.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":p_id", data.value("productId"));
    query.bindValue(":ing_id", data.value("ingredientProductId"));
    query.bindValue(":qty", data.value("quantity").toDouble());
    query.bindValue(":unit", data.value("unitId"));

    if (!query.exec()) {
        qCritical() << "==== DATABASE INSERT ERROR ====";
        qCritical() << "Error Text  :" << query.lastError().text();
        qCritical() << "Full Query  :" << query.executedQuery();

        QVariantList list = query.boundValues();
        for (int i = 0; i < list.size(); ++i) {
            qCritical() << "  " << query.boundValueName(i) << " -> " << list.at(i).toString();
        }
        return false;
    }

    select();
    return true;
}

auto ProductCompositionModel::updateIngredient(const QVariantMap& data) -> bool {
    QSqlQuery query(database());
    // Fixed missing '=' in syntax: quantity_unit_id = :unit
    query.prepare(R"(
        UPDATE product_composition SET
            inventory_id = :ing_id,
            required_quantity = :qty,
            quantity_unit = :unit
        WHERE id = :id
    )");

    query.bindValue(":id", data.value("id"));
    query.bindValue(":ing_id", data.value("ingredientProductId"));
    query.bindValue(":qty", data.value("quantity").toDouble());
    query.bindValue(":unit", data.value("unitId"));

    if (!query.exec()) {
        qCritical() << "==== DATABASE UPDATE ERROR ====";
        qCritical() << "Error Text  :" << query.lastError().text();
        qCritical() << "Full Query  :" << query.executedQuery();

        QVariantList list = query.boundValues();
        for (int i = 0; i < list.size(); ++i) {
            qCritical() << "  " << query.boundValueName(i) << " -> " << list.at(i).toString();
        }
        return false;
    }

    select();
    return true;
}

auto ProductCompositionModel::removeIngredient(const QString& compositionId) -> bool {
    QSqlQuery query(database());
    query.prepare("DELETE FROM product_composition WHERE id = :id");
    query.bindValue(":id", compositionId);

    if (!query.exec()) {
        qWarning() << "Delete Composition Failed:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

[[nodiscard]] auto ProductCompositionModel::compositionAt(int row) const -> ProductComposition {
    if (row < 0 || row >= rowCount()) return {};

    QSqlRecord rec = record(row);
    ProductComposition c;
    c.id                   = rec.value(m_idCol).toString();
    c.productId            = rec.value(m_productCol).toString();
    c.ingredientProductId  = rec.value(m_ingredientCol).toString();
    c.quantity             = rec.value(m_qtyCol).toDouble();
    c.unitId               = rec.value(m_unitCol).toString();

    return c;
}

[[nodiscard]] auto ProductCompositionModel::getAllCompositions() const -> QList<ProductComposition> {
    QList<ProductComposition> compositions;
    compositions.reserve(rowCount());
    for (int i = 0; i < rowCount(); ++i) {
        compositions.append(compositionAt(i));
    }
    return compositions;
}

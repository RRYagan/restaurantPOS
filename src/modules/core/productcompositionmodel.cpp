#include "productcompositionmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QUuid>

ProductCompositionModel::ProductCompositionModel(QObject* parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    setTable("product_composition");
    setEditStrategy(OnManualSubmit);
    select();
    // Initialized based on the provided CREATE TABLE schema
    m_idCol         = fieldIndex("id");
    m_productCol    = fieldIndex("product_id");
    m_ingredientCol = fieldIndex("inventory_id");
    m_qtyCol        = fieldIndex("required_quantity");
    m_unitCol       = fieldIndex("quantity_unit_id");
}


QVariant ProductCompositionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == IngredientNameRole) {
        // Fetch the name of the ingredient from the product table
        QString ingId = QSqlTableModel::data(this->index(index.row(), m_ingredientCol)).toString();
        QSqlQuery query;
        query.prepare("SELECT name FROM inventory WHERE id = :id");
        query.bindValue(":id", ingId);
        if (query.exec() && query.next()) {
            return query.value(0).toString();
        }
        return "Unknown Item";
    }

    switch (role) {
    case IdRole:              return QSqlTableModel::data(this->index(index.row(), m_idCol));
    case ProductIdRole: return QSqlTableModel::data(this->index(index.row(), m_productCol));
    case IngredientIdRole:    return QSqlTableModel::data(this->index(index.row(), m_ingredientCol));
    case QuantityRole:        return QSqlTableModel::data(this->index(index.row(), m_qtyCol));
    case UnitRole:          return QSqlTableModel::data(this->index(index.row(), m_unitCol));
    }

    return QSqlTableModel::data(index, role);
}

QHash<int, QByteArray> ProductCompositionModel::roleNames() const {
    return {
        { IdRole, "id" },
        { ProductIdRole, "productId" },
        { IngredientIdRole, "ingredientId" },
        { IngredientNameRole, "ingredientName" },
        { QuantityRole, "quantity" },
        { UnitRole, "unitId" }
    };
}

QString ProductCompositionModel::productId() const
{
    return m_productId;
}

void ProductCompositionModel::setProductId(const QString& id)
{
    if (m_productId == id)
        return;

    m_productId = id;
    // Updated to match schema column name: product_id
    setFilter(QString("product_id = '%1'").arg(id));
    select();
    emit productIdChanged();
}


bool ProductCompositionModel::addIngredient(const QVariantMap& data)
{
    // QVariantMap c = data;
    // QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // if (c.productId.isEmpty() || c.ingredientProductId.isEmpty()) {
    //     qWarning() << "Invalid composition payload";
    //     return false;
    // }

    QSqlQuery query(database());
    // Updated to use product_id and inventory_product_id
    query.prepare("INSERT INTO product_composition (id, product_id, inventory_id, "
                  "required_quantity, quantity_unit) "
                  "VALUES (:id, :p_id, :ing_id, :qty, :unit)");

    QString finalId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.bindValue(":id", finalId);
    query.bindValue(":p_id", data.value("productId"));
    query.bindValue(":ing_id", data.value("ingredientProductId"));
    query.bindValue(":qty", data.value("quantity"));
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

bool ProductCompositionModel::updateIngredient(const QVariantMap& data)
{
    QSqlQuery query(database());
    // Updated to use inventory_product_id and required_quantity
    query.prepare("UPDATE product_composition SET "
                  "inventory_id = :ing_id, "
                  "required_quantity = :qty,quantity_unit:unit "
                  "WHERE id = :id");

    query.bindValue(":id", data.value("id"));
    // query.bindValue(":p_id", data.value("productId"));
    query.bindValue(":ing_id", data.value("ingredientProductId"));
    query.bindValue(":qty", data.value("quantity"));
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

bool ProductCompositionModel::removeIngredient(const QString& compositionId)
{
    QSqlQuery query(database());
    query.prepare("DELETE FROM product_composition WHERE id = :id");
    query.bindValue(":id", compositionId);

    if (!query.exec()) {
        qWarning() << "Raw Delete Failed:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

QList<ProductComposition> ProductCompositionModel::getAllCompositions() const
{
    QList<ProductComposition> compositions;
    for (int i = 0; i < rowCount(); ++i) {
        compositions.append(compositionAt(i));
    }
    return compositions;
}

ProductComposition ProductCompositionModel::compositionAt(int row) const
{
    ProductComposition c;
    if (row < 0 || row >= rowCount())
        return c;

    QSqlRecord rec = record(row);
    // Explicitly using the schema column names
    c.id                   = rec.value("id").toString();
    c.productId            = rec.value("product_id").toString();
    c.ingredientProductId  = rec.value("inventory_product_id").toString();
    c.quantity             = rec.value("required_quantity").toDouble();
    c.unitId               = rec.value("quantity_unit_id").toString();

    return c;
}

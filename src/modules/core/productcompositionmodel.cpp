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
    // m_idCol = fieldIndex("id");
    // m_productCol = fieldIndex("main_product_item_id");
    // m_ingredientCol = fieldIndex("ingredient_item_id");
    // m_qtyCol = fieldIndex("required_quantity");
    // m_unitCol = fieldIndex("measurement_unit_id");

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
    setFilter(QString("main_product_item_id = '%1'").arg(id));
    select();
    emit productIdChanged();
}

QVariant ProductCompositionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    // Let Qt handle standard roles
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
    }

    const int row = index.row();

    switch (role) {
    case IdRole:
        return QSqlTableModel::data(
            this->index(row, fieldIndex("id"))
            );

    case ProductIdRole:
        return QSqlTableModel::data(
            this->index(row, fieldIndex("main_product_item_id"))
            );

    case IngredientIdRole:
        return QSqlTableModel::data(
            this->index(row, fieldIndex("ingredient_item_id"))
            );

    case QuantityRole:
        return QSqlTableModel::data(
            this->index(row, fieldIndex("required_quantity"))
            );

    case UnitRole:
        return QSqlTableModel::data(
            this->index(row, fieldIndex("measurement_unit_id"))
            );

    default:
        return {};
    }
}


QHash<int, QByteArray> ProductCompositionModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { ProductIdRole, "productId" },
        { IngredientIdRole, "ingredientId" },
        { QuantityRole, "quantity" },
        { UnitRole, "unitId" }
    };
}

// RAW QUERY: Add Ingredient
bool ProductCompositionModel::addIngredient(const ProductComposition& c)
{
    if (c.productId.isEmpty() || c.ingredientProductId.isEmpty()) {
        qWarning() << "Invalid composition payload:"
                   << c.productId << c.ingredientProductId;
        return false;
    }
    if (c.measurementUnitId <= 0) {
        qWarning() << "Invalid unit id";
        return false;
    }
    // Ensure you are using the correct column names from 000_init.up.sql
    QSqlQuery query(database());
    query.prepare("INSERT INTO product_composition (id, main_product_item_id, ingredient_item_id, "
                  "required_quantity, measurement_unit_id) "
                  "VALUES (:id, :p_id, :ing_id, :qty, :unit)");

    QString finalId = c.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : c.id;

    query.bindValue(":id", finalId);
    query.bindValue(":p_id", c.productId);
    query.bindValue(":ing_id", c.ingredientProductId); // Ensure this is not empty/undefined
    query.bindValue(":qty", c.quantity);
    query.bindValue(":unit", c.measurementUnitId);

    if (!query.exec()) {
        qWarning() << "Raw Insert Failed:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

// RAW QUERY: Update Ingredient
bool ProductCompositionModel::updateIngredient(const ProductComposition& c)
{



    QSqlQuery query(database());
    query.prepare("UPDATE product_composition SET "
                  "ingredient_item_id = :ing_id, "
                  "required_quantity = :qty, "
                  "measurement_unit_id = :unit "
                  "WHERE id = :id");



    query.bindValue(":id", c.id);
    query.bindValue(":ing_id", c.ingredientProductId);
    query.bindValue(":qty", c.quantity);
    query.bindValue(":unit", c.measurementUnitId);

    if (!query.exec()) {
        qWarning() << "Raw Update Failed:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

// RAW QUERY: Remove Ingredient
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
    c.id                   = rec.value("id").toString();
    c.productId            = rec.value("main_product_item_id").toString();
    c.ingredientProductId  = rec.value("ingredient_item_id").toString();
    c.quantity             = rec.value("required_quantity").toDouble();
    c.measurementUnitId  = rec.value("measurement_unit_id").toInt();

    return c;
}

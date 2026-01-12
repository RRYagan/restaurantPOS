#include "productcompositionmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

ProductCompositionModel::ProductCompositionModel(QObject* parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    setTable("product_composition");
    setEditStrategy(OnManualSubmit);
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
    setFilter(QString("product_id = '%1'").arg(id));
    select();
    emit productIdChanged();
}

QVariant ProductCompositionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    QSqlRecord rec = record(index.row());

    switch (role) {
    case IdRole:           return rec.value("id");
    case ProductIdRole:    return rec.value("product_id");
    case IngredientIdRole: return rec.value("ingredient_product_id");
    case QuantityRole:     return rec.value("quantity");
    case UnitRole:         return rec.value("measurement_unit_id");
    default:
        return QSqlTableModel::data(index, role);
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

bool ProductCompositionModel::addIngredient(const ProductComposition& c)
{
    if (!c.isValid())
        return false;

    QSqlRecord rec = record();
    rec.setValue("id", c.id);
    rec.setValue("product_id", c.productId);
    rec.setValue("ingredient_product_id", c.ingredientProductId);
    rec.setValue("quantity", c.quantity);
    rec.setValue("measurement_unit_id", c.measurementUnitId);

    if (!insertRecord(-1, rec)) {
        qWarning() << "Insert failed:" << lastError().text();
        return false;
    }
    return submitAll();
}

bool ProductCompositionModel::updateIngredient(const ProductComposition& c)
{
    // 1. Find the row index where the UUID matches
    int rowToUpdate = -1;
    for (int i = 0; i < rowCount(); ++i) {
        if (record(i).value("id").toString() == c.id) {
            rowToUpdate = i;
            break;
        }
    }

    if (rowToUpdate == -1) {
        qWarning() << "Update failed: UUID not found in model" << c.id;
        return false;
    }

    // 2. Get the existing record for that row
    QSqlRecord rec = record(rowToUpdate);

    // 3. Update the values in the record
    rec.setValue("ingredient_product_id", c.ingredientProductId);
    rec.setValue("quantity", c.quantity);
    rec.setValue("measurement_unit_id", c.measurementUnitId);

    // 4. Write the modified record back to the model
    if (!setRecord(rowToUpdate, rec)) {
        qWarning() << "SetRecord failed:" << lastError().text();
        return false;
    }

    return submitAll();
}

// productcompositionmodel.cpp

bool ProductCompositionModel::removeIngredient(const QString& compositionId) {
    for (int i = 0; i < rowCount(); ++i) {
        // Compare as strings for UUIDs
        if (record(i).value("id").toString() == compositionId) {
            removeRow(i);
            return submitAll();
        }
    }
    return false;
}

ProductComposition ProductCompositionModel::compositionAt(int row) const
{
    ProductComposition c;
    if (row < 0 || row >= rowCount())
        return c;

    QSqlRecord rec = record(row);

    c.id                   = rec.value("id").toString();
    c.productId             = rec.value("product_id").toString();
    c.ingredientProductId   = rec.value("ingredient_product_id").toString();
    c.quantity              = rec.value("quantity").toDouble();
    c.measurementUnitId     = rec.value("measurement_unit_id").toInt();

    return c;
}

#include "productmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <QSqlQuery>

ProductModel::ProductModel(QObject* parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    setTable("product");
    setEditStrategy(OnManualSubmit);
    if (!select()) {
        qCritical() << "Select failed for table 'product':" << lastError().text();
    }
    // Debug: Print all found columns
    qDebug() << "Available columns in 'product' table:" << record().fieldName(0) << record().fieldName(1) << record().fieldName(2);
    // In productmodel.cpp constructor
    connect(this, &QSqlTableModel::beforeInsert, [](QSqlRecord &record){
        qDebug() << "Attempting to insert record:" << record;
    });
}


// productmodel.cpp

QVariant ProductModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // 1. If the role is a standard Qt role, use the base class implementation.
    // This handles the calls coming from record() and stops the recursion.
    if (role < Qt::UserRole)
        return QSqlTableModel::data(index, role);

    // 2. Map custom roles to their respective column names/indices
    int column = -1;
    switch (role) {
    case IdRole:       column = fieldIndex("id"); break;
    case KraCodeRole:  column = fieldIndex("kra_unique_item_code"); break;
    case NameRole:     column = fieldIndex("internal_product_name"); break;
    case CategoryRole: column = fieldIndex("product_category_code"); break;
    case PriceRole:    column = fieldIndex("default_selling_price"); break;
    case TaxRole:      column = fieldIndex("tax_classification_id"); break;
    case UnitRole:     column = fieldIndex("measurement_unit_id"); break;
    default:           return QVariant();
    }

    if (column == -1)
        return QVariant();

    // 3. IMPORTANT: Call the base class data() with a specific column index
    // and Qt::DisplayRole. This avoids calling the override again.
    return QSqlTableModel::data(this->index(index.row(), column), Qt::DisplayRole);
}

bool ProductModel::setData(const QModelIndex& index,
                           const QVariant& value,
                           int role)
{
    if (!index.isValid())
        return false;

    QSqlRecord rec = record(index.row());

    switch (role) {
    case NameRole:
        rec.setValue("internal_product_name", value);
        break;
    case PriceRole:
        rec.setValue("default_selling_price", value);
        break;
    default:
        return false;
    }

    return setRecord(index.row(), rec);
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { KraCodeRole, "kraCode" },
        { NameRole, "name" },
        { CategoryRole, "category" },
        { PriceRole, "price" },
        { TaxRole, "taxId" },
        { UnitRole, "unitId" }
    };
}

QList<Product> ProductModel::getAllProducts() const {
    QList<Product> products;
    for (int i = 0; i < rowCount(); ++i) {
        products.append(productAt(i));
    }
    return products;
}

// productmodel.cpp

bool ProductModel::addProduct(const Product &product)
{
    Product p = product;
    // Generate a UUID if one doesn't exist
    QString finalId = p.id.isEmpty() ?
                          QUuid::createUuid().toString(QUuid::WithoutBraces) : p.id;

    QSqlQuery query(database());
    query.prepare("INSERT INTO product (id, kra_unique_item_code, internal_product_name, "
                  "product_category_code, default_selling_price, "
                  "tax_classification_id, measurement_unit_id) "
                  "VALUES (:id, :kra, :name, :cat, :price, :tax, :unit)");

    query.bindValue(":id", finalId);
    query.bindValue(":kra", p.kraUniqueItemCode);
    query.bindValue(":name", p.internalName);
    query.bindValue(":cat", p.categoryCode);
    query.bindValue(":price", p.sellingPrice);
    query.bindValue(":tax", p.taxClassificationId);
    query.bindValue(":unit", p.measurementUnitId);

    if (!query.exec()) {
        qDebug() << "Raw Insert Error:" << query.lastError().text();
        return false;
    }

    // Refresh the model so the UI sees the new record
    select();
    return true;
}

bool ProductModel::removeProduct(const QString& productId)
{
    for (int i = 0; i < rowCount(); ++i) {
        // Changed to .toString() and compare with the UUID string
        if (record(i).value("id").toString() == productId) {
            removeRow(i);
            return submitAll();
        }
    }
    return false;
}

bool ProductModel::updateProduct(const Product& p)
{
    QSqlQuery query(database());
    query.prepare("UPDATE product SET "
                  "internal_product_name = :name, "
                  "default_selling_price = :price, "
                  "tax_classification_id = :tax, "
                  "measurement_unit_id = :unit, "
                  "product_category_code = :cat, "
                  "kra_unique_item_code = :kra "
                  "WHERE id = :id");

    query.bindValue(":id", p.id);
    query.bindValue(":name", p.internalName);
    query.bindValue(":price", p.sellingPrice);
    query.bindValue(":tax", p.taxClassificationId);
    query.bindValue(":unit", p.measurementUnitId);
    query.bindValue(":cat", p.categoryCode);
    query.bindValue(":kra", p.kraUniqueItemCode);

    if (!query.exec()) {
        qDebug() << "Raw Update Error:" << query.lastError().text();
        return false;
    }

    select(); // Refresh UI
    return true;
}
Product ProductModel::productAt(int row) const
{
    Product p;
    if (row < 0 || row >= rowCount())
        return p;

    QSqlRecord rec = record(row);

    p.id                 = rec.value("id").toString();
    p.kraUniqueItemCode   = rec.value("kra_unique_item_code").toString();
    p.internalName        = rec.value("internal_product_name").toString();
    p.categoryCode        = rec.value("product_category_code").toString();
    p.sellingPrice        = rec.value("default_selling_price").toDouble();
    p.taxClassificationId = rec.value("tax_classification_id").toInt();
    p.measurementUnitId   = rec.value("measurement_unit_id").toInt();

    return p;
}

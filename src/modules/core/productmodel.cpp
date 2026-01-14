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
    select();
    // Populate Column Indices based on "CREATE TABLE product"
    m_idCol    = fieldIndex("id");
    m_invCol   = fieldIndex("inventory_product_id");
    m_kraCol   = fieldIndex("kra_unique_item_code");
    m_nameCol  = fieldIndex("internal_product_name");
    m_catCol   = fieldIndex("product_category_id");
    m_typeCol  = fieldIndex("product_type_id");
    m_priceCol = fieldIndex("default_selling_price");
    m_taxCol   = fieldIndex("tax_classification_id");
    m_qtyCol   = fieldIndex("quantity");
    m_unitCol  = fieldIndex("quantity_unit_id");

    if (!select()) {
        qCritical() << "Select failed for table 'product':" << lastError().text();
    }
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { InventoryIdRole, "inventoryId" },
        { KraCodeRole, "kraCode" },
        { NameRole, "name" },
        { CategoryRole, "categoryId" },
        { TypeRole, "typeId" },
        { PriceRole, "price" },
        { TaxRole, "taxId" },
        { QuantityRole, "quantity" },
        { UnitRole, "unitId" }
    };
}

QVariant ProductModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    if (role < Qt::UserRole) return QSqlTableModel::data(index, role);

    const int row = index.row();

    switch (role) {
    case IdRole:          return QSqlTableModel::data(this->index(row, m_idCol));
    case InventoryIdRole: return QSqlTableModel::data(this->index(row, m_invCol));
    case KraCodeRole:     return QSqlTableModel::data(this->index(row, m_kraCol));
    case NameRole:        return QSqlTableModel::data(this->index(row, m_nameCol));
    case CategoryRole:    return QSqlTableModel::data(this->index(row, m_catCol));
    case TypeRole:        return QSqlTableModel::data(this->index(row, m_typeCol));
    case PriceRole:       return QSqlTableModel::data(this->index(row, m_priceCol));
    case TaxRole:         return QSqlTableModel::data(this->index(row, m_taxCol));
    case QuantityRole:    return QSqlTableModel::data(this->index(row, m_qtyCol));
    case UnitRole:        return QSqlTableModel::data(this->index(row, m_unitCol));
    default:              return {};
    }
}

bool ProductModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) return false;
    QSqlRecord rec = record(index.row());

    switch (role) {
    case NameRole:     rec.setValue(m_nameCol, value); break;
    case PriceRole:    rec.setValue(m_priceCol, value); break;
    case QuantityRole: rec.setValue(m_qtyCol, value); break;
    case CategoryRole: rec.setValue(m_catCol, value); break;
    default:           return false;
    }

    if (setRecord(index.row(), rec)) {
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

bool ProductModel::addProduct(const Product &p)
{
    QSqlQuery query(database());
    query.prepare("INSERT INTO product (id, inventory_product_id, kra_unique_item_code, "
                  "internal_product_name, product_category_id, product_type_id, "
                  "default_selling_price, tax_classification_id, quantity, quantity_unit_id) "
                  "VALUES (:id, :inv, :kra, :name, :cat, :type, :price, :tax, :qty, :unit)");

    QString finalId = p.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : p.id;

    query.bindValue(":id", finalId);
    query.bindValue(":inv", p.inventoryProductId);
    query.bindValue(":kra", p.kraUniqueItemCode);
    query.bindValue(":name", p.internalName);
    query.bindValue(":cat", p.categoryCode);
    query.bindValue(":type", p.productTypeId);
    query.bindValue(":price", p.sellingPrice);
    query.bindValue(":tax", p.taxClassificationId);
    query.bindValue(":qty", p.quantity);
    query.bindValue(":unit", p.unitId);

    if (!query.exec()) {
        qWarning() << "Insert Error:" << query.lastError().text();
        return false;
    }
    select();
    return true;
}

bool ProductModel::updateProduct(const Product& p)
{
    QSqlQuery query(database());
    query.prepare("UPDATE product SET inventory_product_id = :inv, internal_product_name = :name, "
                  "default_selling_price = :price, tax_classification_id = :tax, "
                  "quantity_unit_id = :unit, product_category_id = :cat, "
                  "product_type_id = :type, quantity = :qty, kra_unique_item_code = :kra "
                  "WHERE id = :id");

    query.bindValue(":id", p.id);
    query.bindValue(":inv", p.inventoryProductId);
    query.bindValue(":name", p.internalName);
    query.bindValue(":price", p.sellingPrice);
    query.bindValue(":tax", p.taxClassificationId);
    query.bindValue(":unit", p.unitId);
    query.bindValue(":cat", p.categoryCode);
    query.bindValue(":type", p.productTypeId);
    query.bindValue(":qty", p.quantity);
    query.bindValue(":kra", p.kraUniqueItemCode);

    if (!query.exec()) {
        qWarning() << "Update Error:" << query.lastError().text();
        return false;
    }
    select();
    return true;
}

bool ProductModel::removeProduct(const QString& productId)
{
    for (int i = 0; i < rowCount(); ++i) {
        if (record(i).value(m_idCol).toString() == productId) {
            removeRow(i);
            return submitAll();
        }
    }
    return false;
}

QList<Product> ProductModel::getAllProducts() const
{
    QList<Product> products;
    for (int i = 0; i < rowCount(); ++i) {
        products.append(productAt(i));
    }
    return products;
}

Product ProductModel::productAt(int row) const
{
    Product p;
    if (row < 0 || row >= rowCount()) return p;

    QSqlRecord rec = record(row);
    p.id                  = rec.value(m_idCol).toString();
    p.inventoryProductId  = rec.value(m_invCol).toString();
    p.kraUniqueItemCode   = rec.value(m_kraCol).toString();
    p.internalName        = rec.value(m_nameCol).toString();
    p.categoryCode        = rec.value(m_catCol).toString();
    p.productTypeId       = rec.value(m_typeCol).toString();
    p.sellingPrice        = rec.value(m_priceCol).toDouble();
    p.taxClassificationId = rec.value(m_taxCol).toInt();
    p.quantity            = rec.value(m_qtyCol).toInt();
    p.unitId   = rec.value(m_unitCol).toInt();


    return p;
}

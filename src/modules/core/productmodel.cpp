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
    m_taxCol   = fieldIndex("tax_classification_code");
    m_qtyCol   = fieldIndex("quantity");
    m_unitCol  = fieldIndex("quantity_unit_code");
    m_currencyCol = fieldIndex("currency_code");
    m_countryCol  = fieldIndex("country_code");
    m_taxAmtCol   = fieldIndex("tax_amount");

    if (!select()) {
        qCritical() << "Select failed for table 'product':" << lastError().text();
    }
}
QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { InventoryIdRole, "inventoryProductId" }, // Refactored to match QML naming
        { KraCodeRole, "kraUniqueItemCode" },      // Refactored
        { NameRole, "internalProductName" },       // Refactored
        { CategoryRole, "productCategoryId" },     // Refactored
        { TypeRole, "productTypeId" },
        { CurrencyRole, "currencyCode" },            // NEW
        { CountryOriginRole, "countryCode" },  // NEW
        { PriceRole, "defaultSellingPrice" },      // Refactored
        { TaxRole, "taxClassificationCode" },        // Refactored
        { TaxAmountRole, "taxAmount" },            // NEW
        { QuantityRole, "quantity" },
        { UnitRole, "quantityUnitCode" }             // Refactored
    };
}

QVariant ProductModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();

    // Fallback for standard Qt roles (DisplayRole, EditRole, etc.)
    if (role < Qt::UserRole) return QSqlTableModel::data(index, role);

    const int row = index.row();

    switch (role) {
    case IdRole:              return QSqlTableModel::data(this->index(row, m_idCol));
    case InventoryIdRole:     return QSqlTableModel::data(this->index(row, m_invCol));
    case KraCodeRole:         return QSqlTableModel::data(this->index(row, m_kraCol));
    case NameRole:            return QSqlTableModel::data(this->index(row, m_nameCol));
    case CategoryRole:        return QSqlTableModel::data(this->index(row, m_catCol));
    case TypeRole:            return QSqlTableModel::data(this->index(row, m_typeCol));
    case CurrencyRole:        return QSqlTableModel::data(this->index(row, m_currencyCol)); // NEW
    case CountryOriginRole:   return QSqlTableModel::data(this->index(row, m_countryCol));  // NEW
    case PriceRole:           return QSqlTableModel::data(this->index(row, m_priceCol));
    case TaxRole:             return QSqlTableModel::data(this->index(row, m_taxCol));
    case TaxAmountRole:       return QSqlTableModel::data(this->index(row, m_taxAmtCol));   // NEW
    case QuantityRole:        return QSqlTableModel::data(this->index(row, m_qtyCol));
    case UnitRole:            return QSqlTableModel::data(this->index(row, m_unitCol));
    default:
        return QVariant();
    }
}

bool ProductModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) return false;

    QSqlRecord rec = record(index.row());

    switch (role) {
    case IdRole:              rec.setValue(m_idCol, value); break;
    case InventoryIdRole:     rec.setValue(m_invCol, value); break;
    case KraCodeRole:         rec.setValue(m_kraCol, value); break;
    case NameRole:            rec.setValue(m_nameCol, value); break;
    case CategoryRole:        rec.setValue(m_catCol, value); break;
    case TypeRole:            rec.setValue(m_typeCol, value); break;
    case CurrencyRole:        rec.setValue(m_currencyCol, value); break;      // NEW
    case CountryOriginRole:   rec.setValue(m_countryCol, value); break;       // NEW
    case PriceRole:           rec.setValue(m_priceCol, value); break;
    case TaxRole:             rec.setValue(m_taxCol, value); break;
    case TaxAmountRole:       rec.setValue(m_taxAmtCol, value); break;        // NEW
    case QuantityRole:        rec.setValue(m_qtyCol, value); break;
    case UnitRole:            rec.setValue(m_unitCol, value); break;
    default:
        return false;
    }

    // Since EditStrategy is likely OnManualSubmit, this updates the local cache
    if (setRecord(index.row(), rec)) {
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

// In productmodel.cpp

bool ProductModel::addProduct(const QVariantMap &data) {
    QSqlQuery query(database());

    double price = data.value("defaultSellingPrice").toDouble();
    double taxRate = data.value("taxRate").toDouble();
    double taxAmount = price * (taxRate / 100.0);

    // Using unique placeholder names that are NOT substrings of each other
    query.prepare(R"(
        INSERT INTO product (
            id, inventory_product_id, kra_unique_item_code,
            internal_product_name, product_category_id, product_type_id,
            currency_code, country_code, default_selling_price,
            tax_classification_code, tax_amount, quantity, quantity_unit_code
        ) VALUES (
            :p_id, :p_inv, :p_kra, :p_name, :p_cat, :p_type, :p_curr, :p_country,
            :p_price, :p_taxcode, :p_taxamt, :p_qty, :p_unit
        )
    )");

    QString id = data.value("id").toString();
    if (id.isEmpty()) id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Ensure bindValue names match the new unique names exactly
    query.bindValue(":p_id", id);
    query.bindValue(":p_inv", data.value("inventoryProductId"));
    query.bindValue(":p_kra", data.value("kraUniqueItemCode"));
    query.bindValue(":p_name", data.value("internalProductName"));
    query.bindValue(":p_cat", data.value("productCategoryId"));
    query.bindValue(":p_type", data.value("productTypeId"));
    query.bindValue(":p_curr", data.value("currencyCode"));
    query.bindValue(":p_country", data.value("countryCode"));
    query.bindValue(":p_price", price);
    query.bindValue(":p_taxcode", data.value("taxClassificationCode"));
    query.bindValue(":p_taxamt", taxAmount);
    query.bindValue(":p_qty", data.value("quantity").toDouble());
    query.bindValue(":p_unit", data.value("quantityUnitCode"));

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

bool ProductModel::updateProduct(const QVariant &data) {
    QVariantMap p = data.toMap();
    QSqlQuery query(database());

    double price = p.value("defaultSellingPrice").toDouble();
    double taxRate = p.value("taxRate").toDouble();
    double taxAmount = price * (taxRate / 100.0);

    query.prepare(R"(
        UPDATE product
        SET inventory_product_id = :inv,
            kra_unique_item_code = :kra,
            internal_product_name = :name,
            product_category_id = :cat,
            product_type_id = :type,
            currency_code = :curr,
            country_code = :country,
            default_selling_price = :price,
            tax_classification_code = :taxId,
            tax_amount = :taxAmt,
            quantity = :qty,
            quantity_unit_code = :unit,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = :id
    )");

    query.bindValue(":id", p.value("id"));
    query.bindValue(":inv", p.value("inventoryProductId"));
    query.bindValue(":kra", p.value("kraUniqueItemCode"));
    query.bindValue(":name", p.value("internalProductName"));
    query.bindValue(":cat", p.value("productCategoryId"));
    query.bindValue(":type", p.value("productTypeId"));
    query.bindValue(":curr", p.value("currencyCode"));
    query.bindValue(":country", p.value("countryCode"));
    query.bindValue(":price", price);
    query.bindValue(":taxId", p.value("taxClassificationCode"));
    query.bindValue(":taxAmt", taxAmount);
    query.bindValue(":qty", p.value("quantity").toDouble());
    query.bindValue(":unit", p.value("quantityUnitCode"));

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

    select(); // Refresh model cache
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

    // Core Identity
    p.id                  = rec.value(m_idCol).toString();
    p.inventoryProductId  = rec.value(m_invCol).toString();
    p.kraUniqueItemCode   = rec.value(m_kraCol).toString();
    p.internalProductName        = rec.value(m_nameCol).toString();

    // Classifications
    p.productCategoryId        = rec.value(m_catCol).toString();
    p.productTypeId       = rec.value(m_typeCol).toString();

    // New Schema Fields (Mandatory)
    p.currencyCode          = rec.value(m_currencyCol).toString();   // NEW
    p.countryCode = rec.value(m_countryCol).toString();    // NEW

    // Financials
    p.defaultSellingPrice        = rec.value(m_priceCol).toDouble();
    p.taxClassificationCode = rec.value(m_taxCol).toString();
    p.taxAmount           = rec.value(m_taxAmtCol).toDouble();     // NEW

    // Inventory/Stock
    p.quantity            = rec.value(m_qtyCol).toInt();
    p.quantityUnitCode    = rec.value(m_unitCol).toString();

    return p;
}

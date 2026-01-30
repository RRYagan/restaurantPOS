
#include "productmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <QSqlQuery>

ProductModel::ProductModel(QObject* parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
    setTable("product");
    setEditStrategy(OnManualSubmit);

    // Cache indices immediately
    m_idCol       = fieldIndex("id");
    m_kraCol      = fieldIndex("kra_item_code");
    m_nameCol     = fieldIndex("internal_product_name");
    m_catCol      = fieldIndex("product_category_id");
    m_typeCol     = fieldIndex("product_type_id");
    m_priceCol    = fieldIndex("default_selling_price");
    m_taxCol      = fieldIndex("tax_classification_code");
    m_currencyCol = fieldIndex("currency_code");
    m_countryCol  = fieldIndex("country_code");
    m_taxAmtCol   = fieldIndex("tax_amount");

    if (!select()) {
        qCritical() << "Select failed for table 'product':" << lastError().text();
    }
}

auto ProductModel::roleNames() const -> QHash<int, QByteArray>
{
    return {
        { IdRole, "id" },
        { KraCodeRole, "kraItemCode" },
        { NameRole, "internalProductName" },
        { CategoryRole, "productCategoryId" },
        { TypeRole, "productTypeId" },
        { CurrencyRole, "currencyCode" },
        { CountryOriginRole, "countryCode" },
        { PriceRole, "defaultSellingPrice" },
        {PriceFormattedRole, "priceFormatted"},
        { TaxRole, "taxClassificationCode" },
        { TaxAmountRole, "taxAmount" },
        {CategoryIdRole, "categoryId"},
        {ProductTypeIdRole, "productTypeId"}
    };
}

[[nodiscard]] auto ProductModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid()) return {};

    if (role < Qt::UserRole) return QSqlTableModel::data(index, role);

    const int row = index.row();

    switch (role) {
    case IdRole:            {
        return QSqlTableModel::data(this->index(row, m_idCol));
    }
    case KraCodeRole:       {
        return QSqlTableModel::data(this->index(row, m_kraCol));
    }
    case NameRole:          {
        return QSqlTableModel::data(this->index(row, m_nameCol));
    }
    case CategoryRole:      {
        return QSqlTableModel::data(this->index(row, m_catCol));
    }
    case TypeRole:          {
        return QSqlTableModel::data(this->index(row, m_typeCol));

    }
    case CurrencyRole:      {
        return QSqlTableModel::data(this->index(row, m_currencyCol));
    }
    case CountryOriginRole: {
        return QSqlTableModel::data(this->index(row, m_countryCol));
    }
    case PriceRole:        {
        qlonglong cents = QSqlTableModel::data(this->index(row, m_priceCol)).toLongLong();
        return Money(cents).toKSH();
    }
    case PriceFormattedRole :        {
        qlonglong cents = QSqlTableModel::data(this->index(row, m_priceCol)).toLongLong();
        return Money(cents).toKSHString();
    }
    case TaxRole:           {
        return QSqlTableModel::data(this->index(row, m_taxCol));
    }
    case TaxAmountRole:     {
        qlonglong cents = QSqlTableModel::data(this->index(row, m_taxAmtCol)).toLongLong();
        return Money(cents).toKSH();
    }
    default:                {
        return QSqlTableModel::data(index, role);

    }
    }
}

auto ProductModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    if (!index.isValid()) return false;

    QSqlRecord rec = record(index.row());

    switch (role) {
    case IdRole:            rec.setValue(m_idCol, value); break;
    case KraCodeRole:       rec.setValue(m_kraCol, value); break;
    case NameRole:          rec.setValue(m_nameCol, value); break;
    case CategoryRole:      rec.setValue(m_catCol, value); break;
    case TypeRole:          rec.setValue(m_typeCol, value); break;
    case CurrencyRole:      rec.setValue(m_currencyCol, value); break;
    case CountryOriginRole: rec.setValue(m_countryCol, value); break;
    case PriceRole:         rec.setValue(m_priceCol, value); break;
    case TaxRole:           rec.setValue(m_taxCol, value); break;
    case TaxAmountRole:     rec.setValue(m_taxAmtCol, value); break;
    default:                return false;
    }

    if (setRecord(index.row(), rec)) {
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

auto ProductModel::addProduct(const QVariantMap &data) -> bool
{
    QSqlQuery query(database());

    /* money in cents */
    double ksh_value = data.value("defaultSellingPrice").toDouble();
    Money price = Money::toCents(ksh_value);
    double taxRate = data.value("taxRate").toDouble();
    Money taxAmount = price * (taxRate / 100.0);

    query.prepare(R"(
        INSERT INTO product (
            id, kra_item_code, internal_product_name, product_category_id,
            product_type_id, currency_code, country_code, default_selling_price,
            tax_classification_code, tax_amount
        ) VALUES (
            :p_id, :p_kra, :p_name, :p_cat, :p_type, :p_curr, :p_country,
            :p_price, :p_taxcode, :p_taxamt
        )
    )");

    QString id = data.value("id").toString();
    if (id.isEmpty()) id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.bindValue(":p_id", id);
    query.bindValue(":p_kra", data.value("kraItemCode"));
    query.bindValue(":p_name", data.value("internalProductName"));
    query.bindValue(":p_cat", data.value("productCategoryId"));
    query.bindValue(":p_type", data.value("productTypeId"));
    query.bindValue(":p_curr", data.value("currencyCode"));
    query.bindValue(":p_country", data.value("countryCode"));
    query.bindValue(":p_price", static_cast<qint64>(price.cents));
    query.bindValue(":p_taxcode", data.value("taxClassificationCode"));
    query.bindValue(":p_taxamt", static_cast<qint64>(taxAmount.cents));

    if (!query.exec()) {
        qCritical() << "DB Insert Error:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

auto ProductModel::updateProduct(const QVariantMap &data) -> bool
{
    // QVariantMap p = data.toMap();
    QSqlQuery query(database());

    /* money in cents */
    Money price = Money::toCents(data.value("defaultSellingPrice").toDouble());
    double taxRate = data.value("taxRate").toDouble();
    Money taxAmount = price * (taxRate / 100.0);

    query.prepare(R"(
        UPDATE product SET
            kra_item_code = :kra,
            internal_product_name = :name,
            product_category_id = :cat,
            product_type_id = :type,
            currency_code = :curr,
            country_code = :country,
            default_selling_price = :price,
            tax_classification_code = :taxId,
            tax_amount = :taxAmt,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = :id
    )");

    query.bindValue(":id", data.value("id"));
    query.bindValue(":kra", data.value("kraItemCode"));
    query.bindValue(":name", data.value("internalProductName"));
    query.bindValue(":cat", data.value("productCategoryId"));
    query.bindValue(":type", data.value("productTypeId"));
    query.bindValue(":curr", data.value("currencyCode"));
    query.bindValue(":country", data.value("countryCode"));
    query.bindValue(":price", static_cast<qint64>(price.cents));
    query.bindValue(":taxId", data.value("taxClassificationCode"));
    query.bindValue(":taxAmt", static_cast<qint64>(taxAmount.cents));

    if (!query.exec()) {
        qCritical() << "DB Update Error:" << query.lastError().text();
        return false;
    }

    select();
    return true;
}

auto ProductModel::removeProduct(const QString& productId) -> bool
{
    for (int i = 0; i < rowCount(); ++i) {
        if (record(i).value(m_idCol).toString() == productId) {
            removeRow(i);
            return submitAll();
        }
    }
    return false;
}

[[nodiscard]] auto ProductModel::productAt(int row) const -> Product
{
    if (row < 0 || row >= rowCount()) return {};

    QSqlRecord rec = record(row);
    Product p;

    p.id                    = rec.value(m_idCol).toString();
    p.kraItemCode           = rec.value(m_kraCol).toString();
    p.internalProductName   = rec.value(m_nameCol).toString();
    p.productCategoryId     = rec.value(m_catCol).toString();
    p.productTypeId          = rec.value(m_typeCol).toString();
    p.currencyCode          = rec.value(m_currencyCol).toString();
    p.countryCode           = rec.value(m_countryCol).toString();
    p.defaultSellingPrice   = Money(rec.value(m_priceCol).toLongLong());
    p.taxClassificationCode = rec.value(m_taxCol).toString();
    p.taxAmount             = Money(rec.value(m_taxAmtCol).toLongLong());
    return p;
}

[[nodiscard]] auto ProductModel::getAllProducts() const -> QList<Product>
{
    QList<Product> products;
    products.reserve(rowCount());
    for (int i = 0; i < rowCount(); ++i) {
        products.append(productAt(i));
    }
    return products;
}
Product ProductModel::getProductById(const QString &id) const {
    for (int i = 0; i < rowCount(); ++i) {
        // Use the cached column index for ID
        if (record(i).value(m_idCol).toString() == id) {
            return productAt(i); // Returns the full Product struct
        }
    }
    qWarning() << "Product not found in model for ID:" << id;
    return {};
}

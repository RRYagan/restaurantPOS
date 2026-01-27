#include "productmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>

ProductModel::ProductModel(QObject* parent) : QAbstractListModel(parent) {
    loadData();
}

void ProductModel::loadData() {
    beginResetModel();
    m_products.clear();
    QSqlQuery query("SELECT id, kra_item_code, internal_product_name, product_category_id, "
                    "product_type_id, currency_code, country_code, default_selling_price, "
                    "tax_classification_code, tax_amount FROM product");

    while (query.next()) {
        DeepProduct dp;
        dp.product.id = query.value(0).toString();
        dp.product.kraItemCode = query.value(1).toString();
        dp.product.internalProductName = query.value(2).toString();
        dp.product.productCategoryId = query.value(3).toString();
        dp.product.productTypeId = query.value(4).toString();
        dp.product.currencyCode = query.value(5).toString();
        dp.product.countryCode = query.value(6).toString();
        dp.product.defaultSellingPrice = Money(query.value(7).toLongLong());
        dp.product.taxClassificationCode = query.value(8).toString();
        dp.product.taxAmount = Money(query.value(9).toLongLong());

        QSqlQuery compQuery;
        compQuery.prepare("SELECT pc.id, i.name, pc.required_quantity, pc.quantity_unit, pc.inventory_id "
                          "FROM product_composition pc JOIN inventory i ON pc.inventory_id = i.id "
                          "WHERE pc.product_id = :pid");
        compQuery.bindValue(":pid", dp.product.id);
        if (compQuery.exec()) {
            while (compQuery.next()) {
                dp.composition.append(QVariantMap{
                    {"id", compQuery.value(0)}, {"name", compQuery.value(1)},
                    {"quantity", compQuery.value(2)}, {"unit", compQuery.value(3)},
                    {"inventoryId", compQuery.value(4)}
                });
            }
        }
        m_products.push_back(std::move(dp));
    }
    endResetModel();
}

// --- Basic Overrides ---
int ProductModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : m_products.size(); }

QHash<int, QByteArray> ProductModel::roleNames() const {
    return { {IdRole, "id"}, {KraCodeRole, "kraItemCode"}, {NameRole, "internalProductName"},
            {PriceRole, "price"}, {PriceFormattedRole, "priceFormatted"}, {CompositionRole, "availableModifiers"},
            {CategoryIdRole, "categoryId"}, {ProductTypeIdRole, "productTypeId"} };
}

QVariant ProductModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= (int)m_products.size()) return {};
    const auto& dp = m_products[index.row()];
    switch (role) {
    case IdRole: return dp.product.id;
    case NameRole: return dp.product.internalProductName;
    case PriceFormattedRole: {
        // Converts e.g. 15000 to "150.00"
        double p = dp.product.defaultSellingPrice.toKSH();
        return QString::number(p, 'f', 2);
    }
    case CompositionRole: return dp.composition;
    case CategoryIdRole: return dp.product.productCategoryId.toInt();
    default: return {};
    }
}

// --- CRUD Implementation ---

auto ProductModel::addProduct(const QVariantMap &data) -> bool
{
    QSqlQuery query;

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

    // select();
    return true;
}

auto ProductModel::updateProduct(const QVariantMap &data) -> bool
{
    // QVariantMap p = data.toMap();
    QSqlQuery query;


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

    // select();
    return true;
}

bool ProductModel::removeProduct(const QString& productId) {
    QSqlQuery q; q.prepare("DELETE FROM product WHERE id=:id"); q.bindValue(":id", productId);
    if (q.exec()) { loadData(); return true; }
    return false;
}

void ProductModel::syncSingleProduct(const QString& productId) {
    for (size_t i = 0; i < m_products.size(); ++i) {
        if (m_products[i].product.id == productId) {
            QVariantList newComp;
            QSqlQuery q;
            q.prepare("SELECT pc.id, i.name, pc.required_quantity, pc.quantity_unit, pc.inventory_id "
                      "FROM product_composition pc JOIN inventory i ON pc.inventory_id = i.id "
                      "WHERE pc.product_id = :pid");
            q.bindValue(":pid", productId);
            if (q.exec()) {
                while(q.next()) newComp.append(QVariantMap{{"id", q.value(0)}, {"name", q.value(1)}, {"quantity", q.value(2)}, {"unit", q.value(3)}, {"inventoryId", q.value(4)}});
                m_products[i].composition = newComp;
                emit dataChanged(index(i), index(i), {CompositionRole});
            }
            break;
        }
    }
}

bool ProductModel::addIngredient(const QString& productId, const QVariantMap& data) {
    QSqlQuery q;
    q.prepare("INSERT INTO product_composition (id, product_id, inventory_id, required_quantity, quantity_unit) VALUES (:id, :pid, :iid, :qty, :unit)");
    q.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    q.bindValue(":pid", productId);
    q.bindValue(":iid", data["inventoryId"]);
    q.bindValue(":qty", data["quantity"]);
    q.bindValue(":unit", data["unit"]);
    if (q.exec()) { syncSingleProduct(productId); return true; }
    return false;
}

bool ProductModel::updateIngredient(const QString& productId, const QVariantMap& data) {
    QSqlQuery q;
    q.prepare("UPDATE product_composition SET inventory_id=:iid, required_quantity=:qty, quantity_unit=:unit WHERE id=:id");
    q.bindValue(":id", data["id"]);
    q.bindValue(":iid", data["inventoryId"]);
    q.bindValue(":qty", data["quantity"]);
    q.bindValue(":unit", data["unit"]);
    if (q.exec()) { syncSingleProduct(productId); return true; }
    return false;
}

bool ProductModel::removeIngredient(const QString& productId, const QString& compositionId) {
    QSqlQuery q; q.prepare("DELETE FROM product_composition WHERE id=:id"); q.bindValue(":id", compositionId);
    if (q.exec()) { syncSingleProduct(productId); return true; }
    return false;
}

void ProductModel::refresh() { loadData(); }

Product ProductModel::getProductById(const QString &id) const {
    for(const auto& dp : m_products) if(dp.product.id == id) return dp.product;
    return {};
}

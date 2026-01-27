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
    case PriceRole: return dp.product.defaultSellingPrice.toKSH();
    case CompositionRole: return dp.composition;
    case CategoryIdRole: return dp.product.productCategoryId.toInt();
    default: return {};
    }
}

// --- CRUD Implementation ---

bool ProductModel::addProduct(const QVariantMap &data) {
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery q;
    q.prepare("INSERT INTO product (id, internal_product_name, product_category_id, default_selling_price) VALUES (:id, :n, :c, :p)");
    q.bindValue(":id", newId);
    q.bindValue(":n", data["internalProductName"]);
    q.bindValue(":c", data["categoryId"]);
    q.bindValue(":p", (qlonglong)(data["price"].toDouble() * 100));
    if (q.exec()) { loadData(); return true; }
    return false;
}

bool ProductModel::updateProduct(const QVariantMap &data) {
    QSqlQuery q;
    q.prepare("UPDATE product SET internal_product_name=:n, product_category_id=:c, default_selling_price=:p WHERE id=:id");
    q.bindValue(":id", data["id"]);
    q.bindValue(":n", data["internalProductName"]);
    q.bindValue(":c", data["categoryId"]);
    q.bindValue(":p", (qlonglong)(data["price"].toDouble() * 100));
    if (q.exec()) { loadData(); return true; }
    return false;
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

#include "salesviewcontroller.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlError>

SalesViewController::SalesViewController(QObject* parent)
    : QObject(parent)
    , m_orderModel(new OrderModel(this))
    , m_itemModel(new OrderItemModel(this))
{
    // Modern C++11 Signal/Slot connections (Function Pointer Syntax)
    connect(m_itemModel, &OrderItemModel::dataChanged, this, &SalesViewController::orderChanged);
    connect(m_itemModel, &OrderItemModel::rowsInserted, this, &SalesViewController::orderChanged);
    connect(m_itemModel, &OrderItemModel::rowsRemoved, this, &SalesViewController::orderChanged);
}

auto SalesViewController::mapToProduct(const QVariantMap& d) -> Product {
    Product p;
    p.id = d.value("id").toString();
    p.kraItemCode = d.value("kraItemCode").toString();
    p.internalProductName = d.value("internalProductName").toString();
    p.productCategoryId = d.value("productCategoryId").toString();
    p.productTypeId = d.value("productTypeId").toString();
    p.currencyCode = d.value("currencyCode").toString();
    p.countryCode = d.value("countryCode").toString();

    // Explicitly cast from long long (cents) to double to avoid warnings
    p.defaultSellingPrice = static_cast<double>(d.value("price_cents").toLongLong()) / 100.0;
    p.taxClassificationCode = d.value("taxClassificationCode").toString();
    p.taxAmount = d.value("tax_amount").toDouble();

    return p;
}

auto SalesViewController::setIsBusy(bool busy) -> void
{
    if (m_isBusy != busy) {
        m_isBusy = busy;
        emit isBusyChanged();
    }
}


auto SalesViewController::addWithModifiers(const QVariantMap& itemData, const QVariantList& modifiers) -> void
{
    setIsBusy(true);

    // Calculate Final Price (Base Price + Modifiers)
    // Matches the cents logic from your old SalesView.cpp
    const double basePrice = static_cast<double>(itemData.value("price_cents").toLongLong()) / 100.0;
    double extraPrice = 0.0;
    QJsonArray selectedModifiers;

    for (const QVariant& mVar : modifiers) {
        QVariantMap modMap = mVar.toMap();
        const double modPrice = static_cast<double>(modMap.value("price_cents").toLongLong()) / 100.0;
        extraPrice += modPrice;

        QJsonObject modObj;
        modObj.insert("name", modMap.value("name").toString());
        modObj.insert("extra", modPrice);
        selectedModifiers.append(modObj);
    }

    // Map to the Database Model structure
    QVariantMap sqlData;
    sqlData.insert("product_id", itemData.value("id").toString());
    sqlData.insert("quantity", 1);
    sqlData.insert("unit_price", basePrice + extraPrice);

    // Serialize modifiers to JSON for the DB 'modifiers' column
    QJsonDocument doc(selectedModifiers);
    sqlData.insert("modifiers_json", QString(doc.toJson(QJsonDocument::Compact)));

    if (m_itemModel->addOrderItem(sqlData)) {
        emit orderChanged();
    }

    setIsBusy(false);
}

auto SalesViewController::addItem(const QVariantMap& productData) -> bool {
    Product p = mapToProduct(productData);

    StagedItem item;
    item.product = p;
    item.quantity = 1.0;
    item.finalUnitPrice = p.defaultSellingPrice;

    const int nextRow = m_stagedItems.size();

    // FIX: Use the public wrappers
    m_itemModel->prepareForAddition(nextRow);
    m_stagedItems.append(item);
    m_itemModel->finishAddition();

    emit orderChanged();
    return true;
}
auto SalesViewController::removeItem(int index) -> bool {
    if (index >= 0 && index < m_stagedItems.size()) {
        m_itemModel->prepareForRemoval(index);
        m_stagedItems.removeAt(index);
        m_itemModel->finishRemoval();

        emit orderChanged();
        return true;
    }
    return false;
}

auto SalesViewController::updateQuantity(int index, double qty) -> bool {
    if (index >= 0 && index < m_stagedItems.size()) {
        m_stagedItems[index].quantity = qty;

        // Notify the model that this specific row's data changed
        m_itemModel->notifyRowChanged(index, { OrderItemModel::QuantityRole, OrderItemModel::TotalPriceRole });

        emit orderChanged();
        return true;
    }
    return false;
}

auto SalesViewController::makeOrder() -> bool
{
    if (m_stagedItems.isEmpty()) return false;

    setIsBusy(true);
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    m_currentOrderId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QVariantMap orderData;
    orderData.insert("id", m_currentOrderId);
    // Add these if you have them in your UI/Session context
    orderData.insert("table_number", "");
    orderData.insert("waiter_id", "");

    // This now works because addOrder returns a bool
    if (!m_orderModel->addOrder(orderData)) {
        db.rollback();
        setIsBusy(false);
        return false;
    }

    // Using your OrderItemModel to insert individual items
    for (const auto& staged : m_stagedItems) {
        QVariantMap itemData;
        itemData.insert("id", QUuid::createUuid().toString(QUuid::WithoutBraces));
        itemData.insert("order_id", m_currentOrderId);
        itemData.insert("product_id", staged.product.id);
        itemData.insert("quantity", staged.quantity);
        itemData.insert("unit_price", staged.finalUnitPrice);
        itemData.insert("modifiers", staged.modifiersJson);

        if (!m_itemModel->addOrderItem(itemData)) {
            db.rollback();
            setIsBusy(false);
            return false;
        }
    }

    bool success = db.commit();
    setIsBusy(false);
    return success;
}

auto SalesViewController::clearOrder() -> void {
    m_stagedItems.clear();
    m_currentOrderId.clear();
    emit orderChanged();
}

auto SalesViewController::totalFormatted() const -> QString {
    double total = 0.0;
    for (const auto& item : m_stagedItems) total += (item.finalUnitPrice * item.quantity);
    return QString::number(total, 'f', 2);
}


auto SalesViewController::finalizeSale(const QVariantMap& paymentData) -> bool {
    if (m_currentOrderId.isEmpty()) return false;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    double total = 0.0;
    for (const auto& item : m_stagedItems) {
        total += (item.finalUnitPrice * item.quantity);
    }
    const double net = total / 1.16;

    QSqlQuery sq;
    // Using R"()" for cleaner multi-line SQL
    sq.prepare(R"(
        INSERT INTO sale (
            id,
            order_id,
            sale_transaction_date,
            net_amount,
            gross_amount,
            tax_amount,
            total_amount,
            payment_method_id
        ) VALUES (
            :id, :oid, :date, :net, :gross, :tax, :total, :pmid
        )
    )");

    sq.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    sq.bindValue(":oid", m_currentOrderId);
    sq.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
    sq.bindValue(":net", net);
    sq.bindValue(":gross", net);
    sq.bindValue(":tax", total - net);
    sq.bindValue(":total", total);
    sq.bindValue(":pmid", paymentData.value("payment_method_id").toInt());

    if (!sq.exec()) {
        qCritical() << "Finalize Sale Error:" << sq.lastError().text();
        db.rollback();
        return false;
    }

    QSqlQuery uq;
    uq.prepare(R"(
        UPDATE customer_order
        SET order_status = 'closed'
        WHERE id = :id
    )");
    uq.bindValue(":id", m_currentOrderId);

    if (uq.exec() && db.commit()) {
        clearOrder();
        return true;
    }

    db.rollback();
    return false;
}

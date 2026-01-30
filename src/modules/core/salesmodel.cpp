#include "salesmodel.h"
#include <QUuid>
#include <QDateTime>
#include <QSqlError>
#include <algorithm>

SalesModel::SalesModel(QObject *parent) : QAbstractListModel(parent) {}

[[nodiscard]] int SalesModel::rowCount(const QModelIndex &p) const {
    return p.isValid() ? 0 : static_cast<int>(m_items.size());
}

[[nodiscard]] QHash<int, QByteArray> SalesModel::roleNames() const {
    return {
        {NameRole, "name"},
        {QuantityRole, "quantity"},
        {UnitPriceRole, "unitPrice"},
        {TotalPriceRole, "totalPrice"},
        {TaxAmountRole, "taxAmount"},
        {ProductIdRole, "productId"}
    };
}

[[nodiscard]] QVariant SalesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) return {};

    const auto &item = m_items.at(index.row());
    switch (role) {
    case NameRole:        return item.product.internalProductName;
    case QuantityRole:    return item.quantity;
    case UnitPriceRole:   return item.finalUnitPrice.toKSH();
    case TotalPriceRole:  return (item.finalUnitPrice * item.quantity).toKSH();
    case TaxAmountRole:   return (item.taxAmountPerUnit * item.quantity).toKSH();
    case ProductIdRole:   return item.product.id;
    default: return {};
    }
}

// salesmodel.cpp

bool SalesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // 1. Validation
    if (!index.isValid() || index.row() >= m_items.size()) {
        return false;
    }

    auto &item = m_items[index.row()];
    bool changed = false;

    // 2. Handle specific roles
    switch (role) {
    case QuantityRole: {
        double newQty = value.toDouble();
        // Only update if the quantity is valid and actually different
        if (newQty > 0 && item.quantity != newQty) {
            item.quantity = newQty;
            changed = true;
        }
        break;
    }
    case UnitPriceRole: {
        // Useful if you allow manual price overrides for an item
        double newPrice = value.toDouble();
        if (newPrice >= 0 && item.finalUnitPrice.toKSH() != newPrice) {
            item.finalUnitPrice = Money::toCents(newPrice);
            changed = true;
        }
        break;
    }
    default:
        return false;
    }

    // 3. Post-update logic
    if (changed) {
        // Recompute the totals for the whole order (tax, gross, etc.)
        recalculateTotals();

        // Notify the UI that this specific row needs a refresh
        // We include TotalPriceRole because it depends on Quantity/UnitPrice
        emit dataChanged(index, index, {role, TotalPriceRole, TaxAmountRole});

        // Notify the SalesViewController that the grand totals changed
        emit totalsChanged();

        return true;
    }
    if (changed) {
        // This tells the UI to refresh the specific item in the list
        emit dataChanged(index, index, {role});

        // This is the CRITICAL part:
        // You must emit the signal that the Controller uses to notify other screens
        emit orderStatusChanged(); // This triggers kitchenDataChanged in the controller
        return true;
    }

    return false;
}

void SalesModel::addItem(const StagedItem &newItem) {
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [&](const StagedItem &i) { return i.product.id == newItem.product.id; });

    if (it != m_items.end()) {
        it->quantity += newItem.quantity;
        const int row = static_cast<int>(std::distance(m_items.begin(), it));
        auto idx = index(row, 0);
        emit dataChanged(idx, idx, {QuantityRole, TotalPriceRole, TaxAmountRole});
    } else {
        const int row = static_cast<int>(m_items.size());
        beginInsertRows(QModelIndex(), row, row);
        m_items.append(newItem);
        endInsertRows();
        emit countChanged();
    }
    recalculateTotals();
}

void SalesModel::recalculateTotals() {
    Money total(0), tax(0);
    // std::as_const prevents QList "detach" performance penalty
    for (const auto &item : std::as_const(m_items)) {
        total = total + (item.finalUnitPrice * item.quantity);
        tax = tax + (item.taxAmountPerUnit * item.quantity);
    }
    m_cachedTotal = total;
    m_cachedTaxTotal = tax;
    emit totalsChanged();
}

/*QList<KitchenTicket> SalesModel::fetchKitchenQueue() const {
    QList<KitchenTicket> tickets;
    QSqlQuery query;
    query.prepare(R"(
        SELECT co.id, co.table_number, co.created_at,
               GROUP_CONCAT(oi.quantity || 'x ' || p.internal_product_name, '\n') as items
        FROM customer_order co
        JOIN order_item oi ON co.id = oi.order_id
        JOIN product p ON oi.product_id = p.id
        WHERE co.order_status = 'open'
          AND oi.service_state IN ('ordered', 'preparing')
        GROUP BY co.id
        ORDER BY co.created_at ASC
    )");

    if (query.exec()) {
        while (query.next()) {
            tickets.append({
                query.value(0).toString(),
                query.value(1).toString(),
                query.value(2).toDateTime().toString("hh:mm"),
                query.value(3).toString(),
                "" // itemIds placeholder
            });
        }
    }
    return tickets;
}*/

void SalesModel::setTableNumber(const QString &t) {
    if (m_tableNumber != t) {
        m_tableNumber = t;
        emit headerChanged();
    }
}


void SalesModel::removeItem(int index) {
    if (index < 0 || index >= m_items.size()) return;
    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
    recalculateTotals();
    emit countChanged();
}

void SalesModel::updateQuantity(int index, double qty) {
    if (index < 0 || index >= m_items.size()) return;
    m_items[index].quantity = qty;
    emit dataChanged(this->index(index), this->index(index), {QuantityRole, TotalPriceRole});
    recalculateTotals();
}

void SalesModel::clear() {
    beginResetModel();
    m_items.clear();
    m_currentOrderId.clear();
    endResetModel();
    recalculateTotals();
    emit countChanged();
    emit headerChanged();
}

auto SalesModel::loadOrder(const QString &orderId) -> bool {
    QSqlQuery q;
    q.prepare("SELECT table_number, waiter_id FROM customer_order WHERE id = :id");
    q.bindValue(":id", orderId);
    if (!q.exec() || !q.next()) return false;

    beginResetModel();
    m_currentOrderId = orderId;
    m_tableNumber = q.value(0).toString();
    m_waiterId = q.value(1).toString();
    m_items.clear();

    q.prepare("SELECT product_id, quantity, unit_price, tax_amount FROM order_item WHERE order_id = :oid");
    q.bindValue(":oid", orderId);
    if (q.exec()) {
        while (q.next()) {
            StagedItem item;
            item.product.id = q.value(0).toString();
            item.quantity = q.value(1).toDouble();
            item.finalUnitPrice = Money(q.value(2).toLongLong());
            item.taxAmountPerUnit = Money(q.value(3).toLongLong() / q.value(1).toDouble());
            m_items.append(item);
        }
    }
    endResetModel();
    recalculateTotals();
    emit headerChanged();
    emit countChanged();
    return true;
}

auto SalesModel::submitOrder() -> QString {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return "";

    bool isUpdate = !m_currentOrderId.isEmpty();
    QString orderId = isUpdate ? m_currentOrderId : QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery q(db);

    if (isUpdate) {
        q.prepare("UPDATE customer_order SET table_number=:t WHERE id=:id");
    } else {
        q.prepare("INSERT INTO customer_order (id, table_number, waiter_id, order_status) VALUES (:id, :t, :w, 'open')");
        q.bindValue(":w", m_waiterId);
    }
    q.bindValue(":id", orderId);
    q.bindValue(":t", m_tableNumber);
    if (!q.exec()) { db.rollback(); return ""; }

    q.prepare("DELETE FROM order_item WHERE order_id = :id");
    q.bindValue(":id", orderId);
    q.exec();


    q.prepare("INSERT INTO order_item (id, order_id, product_id, quantity, unit_price, tax_amount) VALUES (:id, :oid, :pid, :qty, :p, :tax)");
    for (const auto &i : std::as_const(m_items)) {
        q.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
        q.bindValue(":oid", orderId);
        q.bindValue(":pid", i.product.id);
        q.bindValue(":qty", i.quantity);
        q.bindValue(":p", static_cast<qlonglong>(i.finalUnitPrice.cents));
        q.bindValue(":tax", static_cast<qlonglong>((i.taxAmountPerUnit * i.quantity).cents));
        if (!q.exec()) { db.rollback(); return ""; }
    }

    // Deduct inventory before finishing the transaction
    if (!reduceInventory(db)) {
        qCritical() << "Inventory deduction failed:" << db.lastError().text();
        db.rollback();
        return "";
    }

    if (db.commit()) {
        emit inventorydbModified(); // Notify listeners that stock has changed
        m_currentOrderId = orderId;
        return orderId;
    }
    return "";
}

// void SalesModel::recalculateTotals() {
//     Money total(0), tax(0);
//     for (const auto &i : std::as_const(m_items)) {
//         total = total + (i.finalUnitPrice * i.quantity);
//         tax = tax + (i.taxAmountPerUnit * i.quantity);
//     }
//     m_cachedTotal = total; m_cachedTaxTotal = tax;
//     emit totalsChanged();
// }

// salesmodel.cpp

bool SalesModel::reduceInventory(QSqlDatabase &db) {
    QSqlQuery checkComp(db);
    QSqlQuery updateInv(db);

    for (const auto &item : std::as_const(m_items)) {
        // 1. Query the composition to see what ingredients are used
        checkComp.prepare("SELECT inventory_id, product_id, required_quantity,quantity_unit FROM product_composition WHERE product_id = :pid");
        checkComp.bindValue(":pid", item.product.id);

        if (!checkComp.exec()) return false;

        bool hasComposition = false;
        while (checkComp.next()) {
            hasComposition = true;
            QString componentId = checkComp.value(0).toString();
            double compQtyPerUnit = checkComp.value(2).toDouble();
            double totalToDeduct = compQtyPerUnit * item.quantity;
            qDebug() << "id" << componentId << "total" << totalToDeduct;

            // 2. Update the 'inventory' table using column names from InventoryModel
            updateInv.prepare(R"(
                UPDATE inventory
                SET total_quantity_available = total_quantity_available - :deduct,
                    total_packages_available = CASE
                        WHEN quantity_per_package > 0
                        THEN (total_quantity_available - :deduct) / quantity_per_package
                        ELSE total_packages_available
                    END
                WHERE id = :id
            )");
            updateInv.bindValue(":deduct", totalToDeduct);
            updateInv.bindValue(":id", componentId);

            if (!updateInv.exec()) return false;
        }

        // 3. Fallback: If no composition exists, treat the product itself as the inventory item
        // if (!hasComposition) {
        //     updateInv.prepare(R"(
        //         UPDATE inventory
        //         SET total_quantity_available = total_quantity_available - :qty,
        //             total_packages_available = CASE
        //                 WHEN quantity_per_package > 0
        //                 THEN (total_quantity_available - :qty) / quantity_per_package
        //                 ELSE total_packages_available
        //             END
        //         WHERE id = :pid
        //     )");
        //     updateInv.bindValue(":qty", item.quantity);
        //     updateInv.bindValue(":pid", item.product.id);

        //     if (!updateInv.exec()) return false;
        // }
    }
    return true;
}

[[nodiscard]] QList<Order> SalesModel::fetchAllOrders() const {
    QList<Order> orders;
    QSqlQuery query;

    query.prepare("SELECT id, table_number, waiter_id, order_status, created_at "
                  "FROM customer_order ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            Order order;
            order.id = query.value(0).toString();
            order.tableNumber = query.value(1).toString();
            order.waiterId = query.value(2).toString();
            order.orderStatus = query.value(3).toString();
            order.createdAt = query.value(4).toDateTime();

            // Fetch Items for this specific order
            QSqlQuery itemQuery;
            itemQuery.prepare("SELECT oi.id, p.internal_product_name, oi.quantity, oi.service_state "
                              "FROM order_item oi "
                              "JOIN product p ON oi.product_id = p.id "
                              "WHERE oi.order_id = :orderId");
            itemQuery.bindValue(":orderId", order.id);

            if (itemQuery.exec()) {
                while (itemQuery.next()) {
                    // This now works because 'items' is defined in the struct
                    order.items.append({
                        itemQuery.value(0).toString(),
                        itemQuery.value(1).toString(),
                        itemQuery.value(2).toDouble(),
                        itemQuery.value(3).toString()
                    });
                }
            }
            orders.append(order);
        }
    }
    return orders;
}



bool SalesModel::updateAllStatus(const QString &itemId, const QString &status) {
    QSqlQuery query;
    // General update for all items in an order to a specific service_state (e.g., 'served')
    query.prepare("UPDATE order_item SET service_state = :status WHERE order_id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", itemId);

    if (query.exec()) {
        emit orderStatusChanged();
        return true;
    }
    return false;
}

#include "salesmodel.h"
#include <QUuid>
#include <QDateTime>
#include <QSqlError>
#include <QDebug>


SalesModel::SalesModel(QObject *parent) : QAbstractListModel(parent) {}

[[nodiscrard]]  auto SalesModel::rowCount(const QModelIndex &p) const -> int { return p.isValid() ? 0 : m_items.size(); }

[[nodiscrard]] auto SalesModel::roleNames() const -> QHash<int, QByteArray> {
    return {
            {NameRole, "name"},
            {QuantityRole, "quantity"},
            {UnitPriceRole, "unitPrice"},
            {TotalPriceRole, "totalPrice"},
            {TaxAmountRole, "taxAmount"},
            {ProductIdRole, "productId"} };
}

[[nodiscrard]] auto SalesModel::data(const QModelIndex &index, int role) const -> QVariant {
    if (!index.isValid() || index.row() >= m_items.size()) return {};
    const auto &item = m_items.at(index.row());
    switch (role) {
    case NameRole: return item.product.internalProductName;
    case QuantityRole: return item.quantity;
    case UnitPriceRole: return item.finalUnitPrice.toKSH();
    case TotalPriceRole: return (item.finalUnitPrice * item.quantity).toKSH();
    case ProductIdRole: return item.product.id;
    default: return {};
    }
}

void SalesModel::addItem(const Product &p) {
    // Check if item exists to increment quantity
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [&](const StagedItem &item) { return item.product.id == p.id; });

    if (it != m_items.end()) {
        it->quantity += 1.0;
        int row = static_cast<int>(std::distance(m_items.begin(), it));
        auto idx = index(row, 0);
        emit dataChanged(idx, idx, {QuantityRole, TotalPriceRole, TaxAmountRole});


    } else {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append({
            p,                      // Trusted Product data
            1.0,                    // Quantity
            p.defaultSellingPrice,  // Unit Price (Money)
            "",                     // Modifiers
            p.taxClassificationCode,
            p.taxAmount             // Tax (Money)
        });
        endInsertRows();
        emit countChanged();
    }
    recalculateTotals();
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

void SalesModel::recalculateTotals() {
    Money total(0), tax(0);
    for (const auto &i : std::as_const(m_items)) {
        total = total + (i.finalUnitPrice * i.quantity);
        tax = tax + (i.taxAmountPerUnit * i.quantity);
    }
    m_cachedTotal = total; m_cachedTaxTotal = tax;
    emit totalsChanged();
}

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

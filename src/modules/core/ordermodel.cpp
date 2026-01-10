#include "ordermodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


bool OrderModel::saveOrder(Order &order) {
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.transaction()) return false;

    try {
        QSqlQuery q(db);
        q.prepare("INSERT OR REPLACE INTO orders (id, table_number, status, created_at) VALUES (?, ?, ?, ?)");
        q.addBindValue(order.orderId);
        q.addBindValue(order.tableNumber);
        q.addBindValue(static_cast<int>(order.status));
        q.addBindValue(order.createdAt.toString(Qt::ISODate));
        if (!q.exec()) throw std::runtime_error("Order header failed");

        // Clear items to handle order edits
        QSqlQuery del(db);
        del.prepare("DELETE FROM order_items WHERE order_id = ?");
        del.addBindValue(order.orderId);
        del.exec();

        for (const auto &item : order.items) {
            QSqlQuery iq(db);
            iq.prepare("INSERT INTO order_items (id, order_id, menu_item_id, name, quantity, price_cents) "
                       "VALUES (?, ?, ?, ?, ?, ?)");

            QString itemUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            iq.addBindValue(itemUuid);
            iq.addBindValue(order.orderId);
            iq.addBindValue(item.menuItemId);
            iq.addBindValue(item.name);
            iq.addBindValue(item.quantity);
            iq.addBindValue(static_cast<qlonglong>(item.price.cents));
            if (!iq.exec()) throw std::runtime_error("Item failed");

            // SAVE MODIFIERS
            for (const auto &mod : item.selectModifiers) {
                QSqlQuery mq(db);
                mq.prepare("INSERT INTO order_item_modifiers (order_item_id, modifier_name, extra_price_cents) "
                           "VALUES (?, ?, ?)");
                mq.addBindValue(itemUuid);
                mq.addBindValue(mod.name);
                mq.addBindValue(static_cast<qlonglong>(mod.extraPrice.cents));
                mq.exec();
            }
        }
        return db.commit();
    } catch (...) {
        db.rollback();
        return false;
    }
}


Order OrderModel::loadOrder(const QString& orderId) {
    Order order;
    QSqlQuery q(DatabaseManager::instance().database());

    q.prepare("SELECT table_number, status, created_at FROM orders WHERE id = ?");
    q.addBindValue(orderId);

    if (q.exec() && q.next()) {
        order.orderId = orderId;
        order.tableNumber = q.value(0).toInt();
        order.status = static_cast<OrderStatus>(q.value(1).toInt());
        order.createdAt = QDateTime::fromString(q.value(2).toString(), Qt::ISODate);

        QSqlQuery itemQuery(DatabaseManager::instance().database());
        itemQuery.prepare("SELECT id, menu_item_id, name, quantity, price_cents "
                          "FROM order_items WHERE order_id = ?");
        itemQuery.addBindValue(orderId);

        while (itemQuery.next()) {
            OrderItem item;
            item.id = itemQuery.value(0).toString();
            item.menuItemId = itemQuery.value(1).toInt();
            item.name = itemQuery.value(2).toString();
            item.quantity = itemQuery.value(3).toInt();
            item.price.cents = itemQuery.value(4).toLongLong();
            order.items.append(item);
        }
    }
    return order;
}

int OrderModel::generateOrderId() {
    // Basic implementation: fetch the count and add 1
    QSqlQuery q("SELECT COUNT(*) FROM orders", DatabaseManager::instance().database());
    if (q.next()) {
        return q.value(0).toInt() + 1;
    }
    return 1;
}

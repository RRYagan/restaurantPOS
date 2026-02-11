#include "financemodel.h"
#include <QDebug>
#include <QSqlError>

QVariantList FinanceModel::fetchSalesHistory(const QString &from, const QString &to) {
    QVariantList list;
    QSqlQuery query;

    // Based on your schema:
    // We pull net_amount, tax_amount, and total_amount from the 'sale' table.
    QString sql = "SELECT s.id, s.sale_transaction_date, s.net_amount, s.total_amount, s.tax_amount "
                  "FROM sale s "
                  "WHERE date(s.sale_transaction_date) BETWEEN ? AND ? "
                  "ORDER BY s.sale_transaction_date DESC";

    if (!query.prepare(sql)) {
        qDebug() << "Prepare Error:" << query.lastError().text();
        return list;
    }

    query.addBindValue(from);
    query.addBindValue(to);

    if (!query.exec()) {
        qDebug() << "Execute Error:" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        QVariantMap map;
        map["saleId"] = query.value(0).toString();
        map["date"] = query.value(1).toString();
        map["net_amount"] = query.value(2).toDouble();
        map["total_amount"] = query.value(3).toDouble();
        map["tax_amount"] = query.value(4).toDouble();

        // Temporarily set profit to 0.0 to avoid crashes
        // until we implement the inventory-cost join.
        map["profit"] = 0.0;

        list.append(map);
    }
    return list;
}

QVariantList FinanceModel::fetchOrderComponents(const QString &saleId) {
    QVariantList list;
    QSqlQuery query;

    QString sql =
        "SELECT p.internal_product_name, oi.quantity, oi.unit_price, (oi.quantity * oi.unit_price) as subtotal "
        "FROM order_item oi "
        "JOIN product p ON oi.product_id = p.id "
        "JOIN sale s ON s.order_id = oi.order_id "
        "WHERE s.id = ?";

    query.prepare(sql);
    query.addBindValue(saleId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap map;
            map["name"] = query.value(0).toString();
            map["qty"] = query.value(1).toDouble();
            map["price"] = query.value(2).toDouble();
            map["total"] = query.value(3).toDouble();
            list.append(map);
        }
    }
    return list;
}

// 3. Daily Revenue for Charts
QVariantList FinanceModel::fetchDailyRevenue(int daysBack) {
    QVariantList list;
    QSqlQuery query;

    // Fix: SQLite requires modifiers to be concatenated.
    // Binding "-30" and adding " days" inside the SQL logic.
    query.prepare(R"(
        SELECT date(sale_transaction_date) as day, SUM(net_amount)
        FROM sale
        WHERE sale_transaction_date >= date('now', :days || ' days')
        GROUP BY day
        ORDER BY day ASC
    )");

    // Pass as a negative number (e.g., -30)
    query.bindValue(":days", -daysBack);

    if (!query.exec()) {
        qDebug() << "FetchDailyRevenue Error:" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        QVariantMap map;
        map["date"] = query.value(0);
        map["amount"] = query.value(1);
        list.append(map);
    }
    return list;
}

// 4. Top Profitable Items
QVariantList FinanceModel::fetchTopProfitableItems(int limit) {
    QVariantList list;
    QSqlQuery query;

    query.prepare(R"(
        SELECT p.internal_product_name,
               SUM(oi.quantity) as total_qty,
               SUM(oi.quantity * (oi.unit_price - p.cost_price)) as total_profit
        FROM order_item oi
        JOIN product p ON oi.product_id = p.id
        GROUP BY p.id
        ORDER BY total_profit DESC
        LIMIT :limit
    )");

    // Fix: Ensure the limit is treated as an integer by the driver
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qDebug() << "FetchTopProfitable Error:" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        QVariantMap map;
        map["name"] = query.value(0);
        map["qty"] = query.value(1);
        map["profit"] = query.value(2);
        list.append(map);
    }
    return list;
}

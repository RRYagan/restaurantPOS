#include "salesmodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>

bool SalesModel::recordSale(const SaleRecord &sale) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO sales_revenue_ledger (user_id, sale_transaction_date, gross_total_amount, "
                  "total_tax_amount, net_revenue_amount, kra_receipt_number, kra_digital_signature) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(sale.userId);
    query.addBindValue(sale.saleDate);
    query.addBindValue(static_cast<qint64>(sale.grossAmount.cents));
    query.addBindValue(static_cast<qint64>(sale.taxAmount.cents));
    query.addBindValue(static_cast<qint64>(sale.netRevenue.cents));
    query.addBindValue(sale.kraReceiptNumber);
    query.addBindValue(sale.kraSignature);

    return query.exec();
}

double SalesModel::getTotalRevenue(const QString &startDate, const QString &endDate) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT SUM(net_revenue_amount) FROM sales_revenue_ledger "
                  "WHERE sale_transaction_date BETWEEN ? AND ?");
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

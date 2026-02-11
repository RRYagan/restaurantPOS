#ifndef FINANCEMODEL_H
#define FINANCEMODEL_H

#include <QObject>
#include <QVariantList>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>

class FinanceModel : public QObject {
    Q_OBJECT
public:
    explicit FinanceModel(QObject *parent = nullptr) : QObject(parent) {}

    // Methods required by FinanceController
    QVariantList fetchSalesHistory(const QString &from, const QString &to);
    QVariantList fetchOrderComponents(const QString &saleId);
    QVariantList fetchDailyRevenue(int daysBack);
    QVariantList fetchTopProfitableItems(int limit);
};

#endif

#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QString>
#include <QDateTime>

struct SaleRecord {
    int id;
    int userId;
    QString saleDate; // YYYYMMDD
    double grossAmount;
    double taxAmount;
    double netRevenue;
    long long kraReceiptNumber;
    QString kraSignature;
};

class SalesModel {
public:
    static bool recordSale(const SaleRecord &sale);
    static double getTotalRevenue(const QString &startDate, const QString &endDate);
};

#endif

#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QString>
#include <QDateTime>
#include "types.h"

class SalesModel {
public:
    static bool recordSale(const SaleRecord &sale);
    static double getTotalRevenue(const QString &startDate, const QString &endDate);
};

#endif

#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>

struct Product
{
    QString id;
    QString name;
    QString code;
    QString classCode;
    QString typeCode;
    QString taxType;
    QString pkgUnit;
    QString qtyUnit;
    int basePriceCents = 0;
    QString userId;
    bool isAvailable = true;
};

#endif // PRODUCT_H

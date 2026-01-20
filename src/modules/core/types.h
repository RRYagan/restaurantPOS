#ifndef TYPES_H
#define TYPES_H

#include <QDateTime>


struct Product {
    QString id;
    QString kraItemCode;
    QString internalProductName;
    QString productCategoryId;
    QString productTypeId;
    QString currencyCode;
    QString countryCode;
    double defaultSellingPrice = 0.0;
    QString taxClassificationCode;
    double taxAmount = 0.0;
};

struct ProductComposition {
    QString id;
    QString productId;
    QString ingredientProductId;
    QString unitId;
    double quantity = 0.0;

};

/*inventorymodel */
struct InventoryItem {
    // 1. IDs and Strings
    QString id;
    QString name;
    QString packagingUnitId;
    QString quantityUnitId;
    double quantityPerPackage = 0.0;
    double quantityAvailable = 0.0;
    int32_t packagesAvailable = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct Order {
    QString id;
    QString tableNumber;
    QString waiterId;
    QString orderStatus;
    QDateTime createdAt;
};
/*orderitem */
struct StagedItem {
    Product product;
    double quantity;
    double finalUnitPrice;
    QString modifiersJson;
    QString taxClassificationCode;
    double taxAmountPerUnit;
};

/*usermodel */
struct User {
    int id;
    QString staffIdNumber;
    QString fullName;
    QString userRole;
    QString loginUsername;
    QString passwordHash;
    bool isActive;
};

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


#endif // TYPES_H

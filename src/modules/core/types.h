#ifndef TYPES_H
#define TYPES_H

#include "money.h"

#include <QDateTime>


struct Product {
    QString id;
    QString kraItemCode;
    QString internalProductName;
    QString productCategoryId;
    QString productTypeId;
    QString currencyCode;
    QString countryCode;
    Money defaultSellingPrice;
    QString taxClassificationCode;
    Money taxAmount;
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

struct OrderItem {
    QString itemId;
    QString productName;
    double quantity;
    QString serviceState; // "ordered", "preparing", "served"
};
struct Order {
    QString id;
    QString tableNumber;
    QString waiterId;
    QString orderStatus;
    QDateTime createdAt;

    // Add this member to store the sub-query results
    QList<OrderItem> items;
};
/*orderitem */
struct StagedItem {
    Product product;
    double quantity;
    Money finalUnitPrice;
    QString modifiersJson;
    QString taxClassificationCode;
    Money taxAmountPerUnit;
};
// struct KitchenTicket {
//     QString orderId;
//     QString tableNumber;
//     QString timestamp;
//     QString itemsSummary;
//     QString itemIds;
// };
struct KitchenItem {
    QString id;         // The unique ID from order_item table
    QString name;       // Product name
    double quantity;
    QString status;     // e.g., 'ordered', 'preparing', 'ready'
};

/* The Ticket containing multiple items */
struct KitchenTicket {
    QString orderId;
    QString tableNumber;
    QString timestamp;
    QList<KitchenItem> items; // Changed from itemsSummary string
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
    Money grossAmount;
    Money taxAmount;
    Money netRevenue;
    long long kraReceiptNumber;
    QString kraSignature;
};


#endif // TYPES_H

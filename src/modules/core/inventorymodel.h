#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QString>
#include <QList>

struct InventoryStock {
    int id;
    int productId;
    QString productName;
    double currentQuantity;
    double minimumThreshold;
    QString unitName;
};

class InventoryModel {
public:
    // GET: Retrieve current stock for a specific product
    static InventoryStock getStockByProduct(int productId);

    // GET: Retrieve list of products that are below their minimum threshold (Low Stock Report)
    static QList<InventoryStock> getLowStockAlerts();

    // UPDATE: Manually set a minimum threshold for alerts
    static bool setMinimumThreshold(int productId, double threshold);

    // READ: Get full inventory status for all products
    static QList<InventoryStock> getFullInventoryStatus();
};

#endif

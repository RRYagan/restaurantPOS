#include "inventorymodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

InventoryStock InventoryModel::getStockByProduct(int productId) {
    InventoryStock stock;
    stock.productId = -1;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT s.id, s.current_quantity, s.minimum_threshold, p.internal_product_name, u.unit_display_name "
                  "FROM inventory_stock s "
                  "JOIN product p ON s.product_id = p.id "
                  "JOIN measurement_units u ON p.measurement_unit_id = u.id "
                  "WHERE s.product_id = ?");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        stock.id = query.value(0).toInt();
        stock.productId = productId;
        stock.currentQuantity = query.value(1).toDouble();
        stock.minimumThreshold = query.value(2).toDouble();
        stock.productName = query.value(3).toString();
        stock.unitName = query.value(4).toString();
    }
    return stock;
}

QList<InventoryStock> InventoryModel::getLowStockAlerts() {
    QList<InventoryStock> alerts;
    QSqlQuery query(DatabaseManager::instance().database());

    query.exec("SELECT s.product_id, p.internal_product_name, s.current_quantity, s.minimum_threshold "
               "FROM inventory_stock s "
               "JOIN product p ON s.product_id = p.id "
               "WHERE s.current_quantity <= s.minimum_threshold");

    while (query.next()) {
        InventoryStock item;
        item.productId = query.value(0).toInt();
        item.productName = query.value(1).toString();
        item.currentQuantity = query.value(2).toDouble();
        item.minimumThreshold = query.value(3).toDouble();
        alerts.append(item);
    }
    return alerts;
}

bool InventoryModel::setMinimumThreshold(int productId, double threshold) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO inventory_stock (product_id, minimum_threshold) VALUES (?, ?) "
                  "ON CONFLICT(product_id) DO UPDATE SET minimum_threshold = EXCLUDED.minimum_threshold");
    query.addBindValue(productId);
    query.addBindValue(threshold);
    return query.exec();
}

#include "inventoryviewcontroller.h"
#include "databasemanager.h"
#include <QSqlRecord>
#include <QVariantMap>

InventoryViewController::InventoryViewController(QObject *parent) : QObject(parent) {
    m_inventoryModel = new InventoryModel(this);
    debugInventoryData();

}


void InventoryViewController::debugInventoryData() {
    if (!m_inventoryModel) {
        qWarning() << "Controller: Cannot debug. Model is null.";
        return;
    }

    int rowCount = m_inventoryModel->rowCount();
    qDebug() << "------------------------------------------";
    qDebug() << "CONTROLLER DEBUG: Inventory Table Overview";
    qDebug() << "Total Items in Model:" << rowCount;

    for (int i = 0; i < rowCount; ++i) {
        // Fetch the database record for this row
        QSqlRecord rec = m_inventoryModel->record(i);

        QString id = rec.value("id").toString();
        QString name = rec.value("name").toString();
        double qty = rec.value("quantity_available").toDouble();
        int unitId = rec.value("quantity_unit_id").toInt();

        qDebug() << QString("Row %1 | ID: %2 | Name: %3 | Qty: %4 | UnitID: %5")
                        .arg(i)
                        .arg(id)
                        .arg(name, -15) // Left-aligned name padding
                        .arg(qty)
                        .arg(unitId);
    }
    qDebug() << "------------------------------------------";
}
void InventoryViewController::refresh() {
    if (m_inventoryModel) {
        m_inventoryModel->allItems();
        debugInventoryData(); // Log data after every refresh
    }
}

// Updated: Now accepts a QVariantMap 'data'
bool InventoryViewController::addStock(const QVariantMap &data) {
    if (!m_inventoryModel) return false;

    // We pass the map directly to the model's createItem
    if (m_inventoryModel->createItem(data)) {
        return true;
    }
    return false;
}

// Updated: Now accepts a QVariantMap 'data'
bool InventoryViewController::updateStock(const QVariantMap &data) {
    if (!m_inventoryModel) return false;

    // Ensure the ID is present in the map before sending to model
    if (data.contains("id") && m_inventoryModel->updateItem(data)) {
        return true;
    }
    return false;
}

// Updated: Accepts the ID as a QVariant (string or int)
bool InventoryViewController::deleteStock(const QString& id) {
    if (!m_inventoryModel) return false;

    // Convert the variant ID to String to support your UUID schema
    if (m_inventoryModel->removeItem(id)) {
        return true;
    }
    return false;
}


void InventoryViewController::setInventoryId(const QString& id) {
    qDebug() << "Controller: inventoryId changed from" << m_inventoryId << "to" << id;
    if (m_inventoryId != id) {
        m_inventoryId = id;
        emit inventoryIdChanged();
    }
}

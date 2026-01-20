#include "inventoryviewcontroller.h"
#include "databasemanager.h"
#include <QSqlRecord>
#include <QVariantMap>

InventoryViewController::InventoryViewController(QObject *parent) :
    QObject(parent),
    m_inventoryModel(new InventoryModel(this)) {
    refresh();
}

auto InventoryViewController::inventoryId() const -> QString{
    return m_inventoryId;
}

void InventoryViewController::setInventoryId(const QString& id) {
    qDebug() << "Controller: inventoryId changed from" << m_inventoryId << "to" << id;
    if (m_inventoryModel->inventoryId() != id) {
        m_inventoryModel->setInventoryId(id);
        emit inventoryIdChanged();
    }
}

void InventoryViewController::refresh() {
    if (m_inventoryModel) {
        m_inventoryModel->select(); /* This re-runs the SQL SELECT query */
    }
}

auto InventoryViewController::addStock(const QVariantMap &data) -> bool {
    if (!m_inventoryModel) return false;

    /* We pass the map directly to the model's createItem */
    if (m_inventoryModel->createItem(data)) {
        refresh();
        return true;
    }

    return false;
}

auto InventoryViewController::updateStock(const QVariantMap &data) -> bool {
    if (!m_inventoryModel) return false;

    /* Ensure the ID is present in the map before sending to model */
    if (data.contains("id") && m_inventoryModel->updateItem(data)) {
        refresh();
        return true;
    }
    return false;
}

// Updated: Accepts the ID as a QVariant (string or int)
auto InventoryViewController::deleteStock(const QString& id) -> bool {
    if (!m_inventoryModel) return false;

    /* Convert the variant ID to String to support your UUID schema */
    if (m_inventoryModel->removeItem(id)) {
        refresh();
        return true;
    }
    return false;
}


void InventoryViewController::setCurrentInventoryProduct(const QString& id) {
    m_inventoryModel->setInventoryId(id);
}

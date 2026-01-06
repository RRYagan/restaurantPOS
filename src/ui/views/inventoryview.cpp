#include "inventoryview.h"
#include "databasemanager.h"

InventoryView::InventoryView(QObject *parent) : QObject(parent) {
    m_sourceModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);

    // Wire the source model to the DB fetch function
    m_sourceModel->setDataProvider([]() {
        return DatabaseManager::instance().fetchInventory();
    });

    m_proxy->setSourceModel(m_sourceModel);
}

void InventoryView::refresh() {
    m_sourceModel->refresh();
}

bool InventoryView::addStock(const QString &name, int qty, const QString &unit) {
    if (DatabaseManager::instance().addInventoryItem(name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

bool InventoryView::updateStock(int id, const QString &name, int qty, const QString &unit) {
    if (DatabaseManager::instance().updateInventoryItem(id, name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

bool InventoryView::deleteStock(int id) {
    if (DatabaseManager::instance().deleteInventoryItem(id)) {
        refresh(); // This refreshes the BaseModel and the Proxy
        return true;
    }
    return false;
}

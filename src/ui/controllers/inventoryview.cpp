#include "inventoryview.h"
#include "databasemanager.h"

InventoryView::InventoryView(QObject *parent) : QObject(parent) {
    // Inventory Setup
    m_sourceModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);
    m_sourceModel->setDataProvider([this]() { return m_model.fetchInventory(); });
    m_proxy->setSourceModel(m_sourceModel);

    // History Setup (The Proxy-based approach)
    m_historySourceModel = new BaseModel(this);
    m_historyProxy = new UniversalFilterProxy(this);
    m_historySourceModel->setDataProvider([this]() { return m_model.fetchAllInventoryHistory(); });
    m_historyProxy->setSourceModel(m_historySourceModel);

}

void InventoryView::refresh() {
    m_sourceModel->refresh();
    m_historySourceModel->refresh();
}

bool InventoryView::addStock(const QString &name, int qty, const QString &unit) {
    if (m_model.addInventoryItem(name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

bool InventoryView::updateStock(int id, const QString &name, int qty, const QString &unit) {
    if (m_model.updateInventoryItem(id, name, qty, unit)) {
        refresh();
        return true;
    }
    return false;
}

bool InventoryView::deleteStock(int id) {
    if (m_model.deleteInventoryItem(id)) {
        refresh(); // This refreshes the BaseModel and the Proxy
        return true;
    }
    return false;
}


// QVariantList InventoryView::getHistory(int itemId) {
//     return m_model.fetchInventoryHistory(itemId);
// }

#include "inventoryview.h"
#include "databasemanager.h"

InventoryView::InventoryView(QObject *parent) : QObject(parent) {

}

void InventoryView::refresh() {
    return;
}

bool InventoryView::addStock(const QString &name, int qty, const QString &unit) {
    // if (m_model.addInventoryItem(name, qty, unit)) {
        // refresh();
        // return true;
    // }
    return false;
}

bool InventoryView::updateStock(int id, const QString &name, int qty, const QString &unit) {
    // if (m_model.updateInventoryItem(id, name, qty, unit)) {
    //     refresh();
    //     return true;
    // }
    return false;
}

bool InventoryView::deleteStock(int id) {
    // if (m_model.deleteInventoryItem(id)) {
    //     refresh(); // This refreshes the BaseModel and the Proxy
    //     return true;
    // }
    return false;
}


// QVariantList InventoryView::getHistory(int itemId) {
//     return m_model.fetchInventoryHistory(itemId);
// }

#include "salesviewcontroller.h"

SalesViewController::SalesViewController(QObject* parent)
    : QObject(parent), m_salesModel(new SalesModel(this)), m_productModel(new ProductModel(this)), m_inventoryController(new InventoryViewController(this))
{
    connect(m_salesModel, &SalesModel::totalsChanged, this, &SalesViewController::orderChanged);
    connect(m_salesModel, &SalesModel::countChanged, this, &SalesViewController::itemCountChanged);
    connect(m_salesModel, &SalesModel::inventorydbModified,
            m_inventoryController, &InventoryViewController::refresh);
    connect(m_salesModel, &SalesModel::orderStatusChanged,
            this, &SalesViewController::kitchenDataChanged);
}

bool SalesViewController::addItem(const QString& productId) {
    Product p = m_productModel->getProductById(productId);
    if (p.id.isEmpty()) return false;
    m_salesModel->addItem(p);
    return true;
}

bool SalesViewController::removeItem(int index) {
    m_salesModel->removeItem(index);
    return true;
}

bool SalesViewController::updateQuantity(int index, double qty) {
    m_salesModel->updateQuantity(index, qty);
    return true;
}

void SalesViewController::clearOrder() {
    m_salesModel->clear();
}

bool SalesViewController::makeOrder() {
    m_isBusy = true; emit isBusyChanged();
    QString id = m_salesModel->submitOrder();
    m_isBusy = false; emit isBusyChanged();
    return !id.isEmpty();
}

bool SalesViewController::editOrder(const QString& orderId) {
    return m_salesModel->loadOrder(orderId);
}

QVariantList SalesViewController::loadOrders() {
    QVariantList list;
    auto orders = m_salesModel->fetchAllOrders();
    for (const auto &o : orders) {
        QVariantMap map;
        map["orderId"] = o.id;
        map["displayTitle"] = "Table " + o.tableNumber;
        map["status"] = o.orderStatus;
        map["date"] = o.createdAt.toString("yyyy-MM-dd hh:mm");
        list.append(map);
    }
    return list;
}

QVariantList SalesViewController::getKitchenQueue() {
    QVariantList result;
    auto queue = m_salesModel->fetchKitchenQueue();
    for (const auto &t : queue) {
        QVariantMap map;
        map["orderId"] = t.orderId;
        map["tableNumber"] = t.tableNumber;
        map["time"] = t.timestamp;
        map["items"] = t.itemsSummary;
        result.append(map);
    }
    return result;
}

void SalesViewController::updateItemStatus(const QString &orderId, const QString &status) {
    m_salesModel->updateItemStatus(orderId, status);
}

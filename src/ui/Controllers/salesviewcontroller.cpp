#include "salesviewcontroller.h"

SalesViewController::SalesViewController(QObject* parent)
    : QObject(parent), m_salesModel(new SalesModel(this)), m_productModel(new ProductModel(this)), m_inventoryController(new InventoryViewController(this))
{
    connect(m_salesModel, &SalesModel::totalsChanged, this, &SalesViewController::orderChanged);
    connect(m_salesModel, &SalesModel::countChanged, this, &SalesViewController::itemCountChanged);
    connect(m_salesModel, &SalesModel::inventorydbModified,
            m_inventoryController, &InventoryViewController::refresh);
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

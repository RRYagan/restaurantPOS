#include "orderviewcontroller.h"

OrderViewController::OrderViewController(QObject *parent) : QObject(parent)
{
    // Access the database via your singleton
    m_orderModel = new OrderTableModel(this, DatabaseManager::instance().database());
}

void OrderViewController::createNewOrder(int tableNum, int amount) {
    // Business logic: currently fixed waiter_id for example
    m_orderModel->addOrder(tableNum, "WAITER_01", amount);
}

void OrderViewController::markAsPaid(const QString &orderId) {
    m_orderModel->updateOrderStatus(orderId, "PAID");
}

void OrderViewController::refresh() {
    m_orderModel->select();
}

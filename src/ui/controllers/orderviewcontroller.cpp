#include "orderviewcontroller.h"
#include <QUuid>

OrderViewController::OrderViewController(QObject* parent)
    : QObject(parent),
    m_model(new OrderTableModel(this)),
    m_locale(QLocale::English, QLocale::UnitedStates)

{
    connect(m_model, &OrderTableModel::totalsChanged, this, &OrderViewController::orderChanged);
}

// orderviewcontroller.cpp
void OrderViewController::addItem(const QVariantMap& itemMap)
{
    // Convert the QVariantMap from QML into our C++ Struct
    OrderItem item;
    item.menuItemId = itemMap["id"].toString();
    item.name = itemMap["name"].toString();
    item.unitPriceCents = itemMap["price"].toInt(); // Ensure this matches your QML property name
    item.taxType = itemMap["taxType"].toString();
    item.quantity = 1.0;

    // Pass the struct to the model
    m_model->addItem(item);
    emit orderChanged();
}

void OrderViewController::removeItem(int index)
{
    m_model->removeItem(index);
    emit orderChanged();
}

void OrderViewController::clearOrder()
{
    m_model->clearCurrentOrder();
    emit orderChanged();
}

int OrderViewController::itemCount() const
{
    return m_model->rowCount();
}

QString OrderViewController::totalFormatted() const
{
    // Convert cents to decimal string (e.g., 1050 -> "10.50")
    double total = m_model->currentTotalCents() / 100.0;
    return m_locale.toString(total, 'f', 2);
}

bool OrderViewController::makeOrder()
{
    if (m_model->rowCount() == 0) return false;

    // Submit to DB (logic handles orders + order_items tables)
    if (m_model->submitOrder()) {
        clearOrder();
        return true;
    }
    return false;
}

void OrderViewController::updateQuantity(int index, double newQuantity)
{
    // 1. If quantity is reduced to 0 or less, remove the item entirely
    if (newQuantity <= 0) {
        removeItem(index);
        return;
    }

    // 2. Update the value in the underlying TableModel
    // This assumes you added updateQuantity(int, double) to OrderTableModel
    m_model->updateQuantity(index, newQuantity);

    // 3. Emit the signal that refreshes 'totalFormatted' and 'itemCount' in the UI
    emit orderChanged();
}

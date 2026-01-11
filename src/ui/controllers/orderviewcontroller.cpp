#include "orderviewcontroller.h"
#include <QUuid>

OrderViewController::OrderViewController(QObject* parent)
    : QObject(parent),
    m_model(new OrderTableModel(this)),
    m_locale(QLocale::English, QLocale::UnitedStates)

{
    // connect(m_model, &OrderTableModel::totalsChanged, this, &OrderViewController::orderChanged);
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

    for (int i = 0; i < m_currentItems.size(); i++) {
        if (m_currentItems[i].menuItemId == item.menuItemId) {
            m_currentItems[i].quantity += 1;

            // Tell the internal model to reload data from the updated m_items
            // m_model->refresh();

            return;
        }
    }

    // Pass the struct to the model
    m_currentItems.append((item));
    calculateTotal();
    // m_model->addItem(item);
    qDebug() << "Added item name:" << m_currentItems.last().name
        << "Quantity:" << m_currentItems.last().quantity;
    totalsChanged();
    emit orderChanged();
}

void OrderViewController::removeItem(int index)
{
    if (m_currentItems.size() > 0) {
        m_currentItems.removeAt(index);
    // m_model->removeItem(index);
    }
    totalsChanged();
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
    totalsChanged();
    // 3. Emit the signal that refreshes 'totalFormatted' and 'itemCount' in the UI
    emit orderChanged();
}

void OrderViewController::calculateTotal() {
    int64_t total = 0;
    for (const auto& item : m_currentItems) {
        // Base price * quantity
        int64_t itemTotal = item.unitPriceCents;

        // Add all selected modifiers
        // for (const auto& mod : item.selectModifiers) {
        //     itemTotal += mod.extraPrice.cents;
        // }

        total += (itemTotal * item.quantity);
    }
    m_currentTotalCents = total;
    emit totalsChanged();
}
QString OrderViewController::totalFormatted() const
{
    double total = m_currentTotalCents / 100.0;
    return m_locale.toString(total, 'f', 2);
}

#include "salesview.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <order.h>
#include <basemodel.h>

SalesView::SalesView(QObject *parent) : QObject(parent) {
    m_internalModel = new BaseModel(this);
    m_proxy = new UniversalFilterProxy(this);

    // Link: SalesView provides data -> BaseModel stores it -> Proxy filters it
    m_internalModel->setDataProvider([this]() { return getSalesData(); });
    m_proxy->setSourceModel(m_internalModel);
}
QVariantList SalesView::getSalesData() {
    QVariantList list;
    for (const auto& item : m_items) {
        QVariantMap map;
        map["id"] = item.id;
        map["menuItemId"] = item.menuItemId;
        map["name"] = item.name;
        map["quantity"] = item.quantity;
        map["price"] = QVariant::fromValue(item.price);
        // --- ADD THIS SECTION TO FIX THE ISSUE ---
        QVariantList modsList;
        for (const auto& mod : item.selectModifiers) {
            QVariantMap modMap;
            modMap["name"] = mod.name;
            // Ensure the key matches what you use in QML (price_cents)
            modMap["price_cents"] = static_cast<qlonglong>(mod.extraPrice.cents);
            modsList.append(modMap);
        }
        map["modifiers"] = modsList; // Map the list to the "modifiers" key used in QML
        // ------------------------------------------
        list.append(map);
    }
    return list;
}


// --- Cart Actions ---

void SalesView::addItemToOrder(int menuItemId) {
    // 1. Check if the item already exists in the cart
    for (int i = 0; i < m_items.size(); i++) {
        if (m_items[i].menuItemId == menuItemId) {
            m_items[i].quantity += 1;

            // Tell the internal model to reload data from the updated m_items
            m_internalModel->refresh();
            calculateTotal();
            return;
        }
    }

    // 2. If not found, fetch item details from the database
    QSqlQuery query;
    query.prepare("SELECT name, base_price_cents FROM menu_items WHERE id = ?");
    query.addBindValue(menuItemId);

    if (query.exec() && query.next()) {
        OrderItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.menuItemId = menuItemId;
        item.name = query.value("name").toString();
        item.quantity = 1;
        item.price.cents = query.value("base_price_cents").toLongLong();

        // Add to our local list
        m_items.append(item);

        // 3. Refresh the model so the Proxy and QML see the new item
        m_internalModel->refresh();
        calculateTotal();
    } else {
        qDebug() << "Failed to find menu item with ID:" << menuItemId;
    }
}
void SalesView::removeItem(int proxyIndex) {
    QModelIndex pIdx = m_proxy->index(proxyIndex, 0);
    QModelIndex sIdx = m_proxy->mapToSource(pIdx);

    if (sIdx.isValid() && sIdx.row() < m_items.size()) {
        m_items.removeAt(sIdx.row());
        m_internalModel->refresh(); // Mandatory to update UI
        calculateTotal();
    }
}
void SalesView::updateQuantity(int proxyIndex, int newQuantity) {
    // 1. Map the Proxy Index from QML to the Source Model Index
    QModelIndex pIdx = m_proxy->index(proxyIndex, 0);
    QModelIndex sIdx = m_proxy->mapToSource(pIdx);

    // 2. Validate the mapped index
    if (!sIdx.isValid() || sIdx.row() < 0 || sIdx.row() >= m_items.size() || newQuantity <= 0) {
        return;
    }

    // 3. Update the data in the underlying list
    m_items[sIdx.row()].quantity = newQuantity;

    // 4. Refresh the internal model to push changes to the UI
    m_internalModel->refresh();

    // 5. Recalculate the order total
    calculateTotal();
}

void SalesView::clearOrder() {
    // 1. Clear the underlying data list
    m_items.clear();

    // 2. Reset associated state
    m_currentOrderId = "";
    m_totalMoney.cents = 0;

    // 3. Notify the internal model to refresh (this updates the UI/Proxy)
    m_internalModel->refresh();

    // 4. Notify QML properties that state has changed
    emit currentOrderIdChanged();
    emit totalChanged();
}


// --- Database & Utility ---

bool SalesView::makeOrder() {
    if (m_items.isEmpty()) return false;

    m_isBusy = true;
    emit isBusyChanged();

    Order order;
    order.orderId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    order.items = m_items;
    order.tableNumber = 1;
    order.status = OrderStatus::Open;
    order.createdAt = QDateTime::currentDateTime();

    bool success = m_model.saveOrder(order);

    m_isBusy = false;
    emit isBusyChanged();
    return success;
}

void SalesView::calculateTotal() {
    int64_t total = 0;
    for (const auto& item : m_items) {
        // Base price * quantity
        int64_t itemTotal = item.price.cents;

        // Add all selected modifiers
        for (const auto& mod : item.selectModifiers) {
            itemTotal += mod.extraPrice.cents;
        }

        total += (itemTotal * item.quantity);
    }
    m_totalMoney.cents = total;
    emit totalChanged();
}

QString SalesView::totalFormatted() const {
    return m_totalMoney.toString();
}

// salesview.cpp

void SalesView::addWithModifiers(const QVariantMap &itemData, const QVariantList &modifiers) {
    OrderItem item;
    // Generate a unique ID for this specific line item in the cart
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.menuItemId = itemData["id"].toInt();
    item.name = itemData["name"].toString();
    item.price.cents = itemData["price_cents"].toLongLong();
    item.quantity = 1;

    // Map selected QML modifiers to the C++ struct
    for (const QVariant &mVar : modifiers) {
        QVariantMap mData = mVar.toMap();
        Modifier mod;
        mod.id = mData["id"].toInt();
        mod.name = mData["name"].toString();
        mod.extraPrice.cents = mData["price_cents"].toLongLong();
        item.selectModifiers.append(mod);
    }

    // Add to the local list used by the UI
    m_items.append(item);

    // Refresh the internal model so the GridView/ListView updates
    m_internalModel->refresh();
    calculateTotal();
}

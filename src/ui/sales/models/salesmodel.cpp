#include "salesmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <order.h>
#include <databasemanager.h>

SalesModel::SalesModel(QObject *parent) : QAbstractTableModel(parent) {}

int SalesModel::columnCount(const QModelIndex &parent) const {
    return 3; // Quantity, Name, Price
}

int SalesModel::rowCount(const QModelIndex &parent) const {
    // Determine row count based on the current active view mode
    return m_items.size();
}

QVariant SalesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    // Select the source list based on state
    const QList<OrderItem> &currentList = m_items;

    if (index.row() >= currentList.size())
        return QVariant();

    const OrderItem &item = currentList.at(index.row());

    switch (role) {
    case QuantityRole: return item.quantity;
    case NameRole:     return item.name;
    case PriceRole:    return QVariant::fromValue(item.price);
    case ItemIdRole:   return item.id;
    case MenuIdRole:   return item.menuItemId;
    }
    return QVariant();
}

QHash<int, QByteArray> SalesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[QuantityRole] = "quantity";
    roles[NameRole] = "name";
    roles[PriceRole] = "price";
    roles[ItemIdRole] = "itemId";
    roles[MenuIdRole] = "menuId";
    return roles;
}

// --- Cart Actions ---

void SalesModel::addItemToOrder(int menuItemId) {
    for (int i = 0; i < m_items.size(); i++) {
        if (m_items[i].menuItemId == menuItemId) {
            m_items[i].quantity += 1;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {QuantityRole});
            calculateTotal();
            return;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT name, base_price_cents FROM menu_items WHERE id = ?");
    query.addBindValue(menuItemId);

    if (query.exec() && query.next()) {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());

        OrderItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.menuItemId = menuItemId;
        item.name = query.value("name").toString();
        item.quantity = 1;
        item.price.cents = query.value("base_price_cents").toLongLong();
        m_items.append(item);

        endInsertRows();
        calculateTotal();
    }
}

void SalesModel::removeItem(int index) {
    if (index < 0 || index >= m_items.size()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();

    calculateTotal();
}

void SalesModel::updateQuantity(int index, int newQuantity) {
    if (index < 0 || index >= m_items.size() || newQuantity <= 0) return;

    m_items[index].quantity = newQuantity;
    QModelIndex idx = this->index(index, 0);
    emit dataChanged(idx, idx, {QuantityRole});

    calculateTotal();
}

void SalesModel::clearOrder() {
    beginResetModel();
    m_items.clear();
    m_currentOrderId = "";
    m_totalMoney.cents = 0;
    endResetModel();

    emit currentOrderIdChanged();
    emit totalChanged();
}


// --- Database & Utility ---

bool SalesModel::makeOrder() {
    if (m_items.isEmpty()) return false;

    m_isBusy = true;
    emit isBusyChanged();

    Order order;
    order.orderId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    order.items = m_items;
    order.tableNumber = 1;
    order.status = OrderStatus::Open;
    order.createdAt = QDateTime::currentDateTime();

    bool success = DatabaseManager::instance().saveOrder(order);

    if (success) {
        clearOrder();
    }

    m_isBusy = false;
    emit isBusyChanged();
    return success;
}

void SalesModel::calculateTotal() {
    int64_t total = 0;
    // Calculate total based on whichever list is currently being viewed
    const QList<OrderItem> &currentList = m_items;

    for (const auto& item : currentList) {
        total += (item.price.cents * item.quantity);
    }
    m_totalMoney.cents = total;
    emit totalChanged();
}

QString SalesModel::totalFormatted() const {
    return m_totalMoney.toString();
}

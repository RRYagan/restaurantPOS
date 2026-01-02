#include "salesmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <order.h>
#include <databasemanager.h>

SalesModel::SalesModel(QObject *parent) : QAbstractTableModel(parent) {}

int SalesModel::rowCount(const QModelIndex &parent) const {
    return m_items.size();
}

int SalesModel::columnCount(const QModelIndex &parent) const {
    return 3; // Quantity, Name, Price
}

QVariant SalesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const OrderItem &item = m_items.at(index.row());

    switch (role) {
    case QuantityRole:
        return item.quantity; // Maps to "quantity" INTEGER
    case NameRole:
        return item.name;     // Maps to "name" TEXT
    case PriceRole:
        // Returns the Money gadget for formatted display in QML
        return QVariant::fromValue(item.price);
    case ItemIdRole:
        return item.id;       // The UUID string for order_items.id
    case MenuIdRole:
        return item.menuItemId; // The INTEGER FK for menu_items
    }
    return QVariant();
}

QHash<int, QByteArray> SalesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    // These names are used directly in MenuScreen.qml delegates
    roles[QuantityRole] = "quantity";
    roles[NameRole] = "name";
    roles[PriceRole] = "price";
    roles[ItemIdRole] = "itemId";   // The unique UUID for this row
    roles[MenuIdRole] = "menuId";   // The ID linking back to the Menu
    return roles;
}

void SalesModel::clearOrder() {

    beginResetModel();
    m_items.clear();
    m_currentOrderId = "";
    m_totalMoney = Money();
    endResetModel();

    emit currentOrderIdChanged();
    emit totalChanged();
}

void SalesModel::addItemToOrder(int menuItemId) {
    // Check if item exists to increment quantity
    for (int i = 0; i < m_items.size(); i++) {
        if (m_items[i].menuItemId == menuItemId) {
            m_items[i].quantity += 1;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {QuantityRole});
            calculateTotal();
            return;
        }
    }

    // Fetch details from DB via a standardized query (could also be moved to DatabaseManager)
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



// In salesmodel.cpp
bool SalesModel::makeOrder() {
    if (m_items.isEmpty()) return false;

    // Create the order object to pass to the DB manager
    Order order;
    order.orderId = m_currentOrderId; // If empty, DBManager generates one. If not, it updates.
    order.items = m_items;
    order.tableNumber = 1;
    order.status = OrderStatus::Open;
    order.createdAt = QDateTime::currentDateTime();

    // The logic is now encapsulated in DatabaseManager
    if (DatabaseManager::instance().saveOrder(order)) {
        clearOrder();
        return true;
    }

    return false;
}
void SalesModel::refresh() {
    if (m_currentOrderId == "") return;

    // 1. Tell the view we are about to change everything
    beginResetModel();

    m_items.clear();

    QSqlQuery query;
    query.prepare("SELECT name, quantity, price_cents "
                  "FROM order_items "
                  "WHERE order_id = ?");
    query.addBindValue(m_currentOrderId);

    if (query.exec()) {
        while (query.next()) {
            OrderItem item;
            item.name = query.value(0).toString();
            item.quantity = query.value(1).toInt();
            item.price.cents = query.value(2).toLongLong();
            m_items.append(item);
        }
    } else {
        qDebug() << "Refresh Error:" << query.lastError().text();
    }

    // 2. Tell the view we are done and it should redraw
    endResetModel();

    // 3. Update the total at the bottom
    calculateTotal();
}

void SalesModel::calculateTotal() {
    int64_t total = 0;
    for (const auto& item : m_items) {
        total += (item.price.cents * item.quantity);
    }
    m_totalMoney.cents = total;
    emit totalChanged();
}

QString SalesModel::totalFormatted() const {
    return m_totalMoney.toString();
}


void SalesModel::switchToCart() {
    if (!m_isShowingHistory) return;

    beginResetModel();
    m_items = m_activeCart; // Restore the saved cart
    m_isShowingHistory = false;
    endResetModel();
    calculateTotal();
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

void SalesModel::viewOrderDetails(const QString& orderId) {
    m_currentOrderId = orderId; // Store the ID
    emit currentOrderIdChanged();

    beginResetModel();
    m_items.clear();

    QSqlQuery query;

    query.prepare("SELECT name, quantity, price_cents FROM order_items WHERE order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            OrderItem item;
            item.name = query.value(0).toString();
            item.quantity = query.value(1).toInt();
            item.price.cents = query.value(2).toLongLong();
            m_items.append(item);
        }
    }
    endResetModel();
    calculateTotal();
}

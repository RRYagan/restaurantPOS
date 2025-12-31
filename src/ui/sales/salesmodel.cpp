#include "salesmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>


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
        return item.quantity; // Maps to "quantity" INTEGER [cite: 1, 18, 20]
    case NameRole:
        return item.name;     // Maps to "name" TEXT [cite: 1, 18, 20]
    case PriceRole:
        // Returns the Money gadget for formatted display in QML [cite: 1, 18, 23, 24]
        return QVariant::fromValue(item.price);
    case ItemIdRole:
        return item.id;       // The UUID string for order_items.id
    case MenuIdRole:
        return item.menu_item_id; // The INTEGER FK for menu_items
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

bool SalesModel::startNewOrder(int menuItemId) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return false;

    QSqlQuery query;

    // 1. Insert into orders - explicitly naming columns
    query.prepare("INSERT INTO orders (table_number, status) VALUES (?, 'OPEN')");
    query.addBindValue(1); // Defaulting to Table 1 for now

    if (!query.exec()) {
        qDebug() << "Order Insert Error:" << query.lastError().text();
        db.rollback();
        return false;
    }

    // Get the new Order ID
    int orderId = query.lastInsertId().toInt();

    // 2. Fetch Item Details from menu_items
    QSqlQuery itemQuery;
    itemQuery.prepare("SELECT name, base_price_cents FROM menu_items WHERE id = ?");
    itemQuery.addBindValue(menuItemId);

    if (!itemQuery.exec() || !itemQuery.next()) {
        qDebug() << "Menu Item Not Found:" << itemQuery.lastError().text();
        db.rollback();
        return false;
    }

    QString name = itemQuery.value("name").toString();
    qlonglong price = itemQuery.value("base_price_cents").toLongLong();

    // 3. Insert into order_items with a UUID
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare("INSERT INTO order_items (id, order_id, menu_item_id, name, quantity, price_cents) "
                  "VALUES (?, ?, ?, ?, 1, ?)");
    query.addBindValue(uuid);
    query.addBindValue(orderId);
    query.addBindValue(menuItemId);
    query.addBindValue(name);
    query.addBindValue(static_cast<qlonglong>(price));

    if (!query.exec()) {
        qDebug() << "Order Item Insert Error:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (db.commit()) {
        m_currentOrderId = orderId;
        refresh(); // Refresh the QAbstractTableModel
        return true;
    }
    return false;
}

// Add the missing clearOrder implementation to fix the build error
void SalesModel::clearOrder() {
    beginResetModel();
    m_items.clear();
    m_currentOrderId = -1;
    endResetModel();
    emit totalChanged();
}
void SalesModel::addItemToOrder(int menuItemId) {
    for (int i = 0; i < m_items.size(); i++) {
        if (m_items[i].menu_item_id == menuItemId) {
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
        //  We use beginInsertRows to notify the UI to show a new line immediately
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());

        OrderItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.menu_item_id = menuItemId; //  Store for final SQL insert
        item.name = query.value("name").toString();
        item.quantity = 1;
        item.price.cents = query.value("base_price_cents").toLongLong();
        m_items.append(item);

        endInsertRows();
        calculateTotal();
    }
}

bool SalesModel::makeOrder() {
    if (m_items.isEmpty()) return false;



    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return false;

    QSqlQuery query;
    // 1. Parent Order: Table Number and Status
    query.prepare("INSERT INTO orders (table_number, status) VALUES (?, 'OPEN')");
    query.addBindValue(1);

    if (!query.exec()) {
        qDebug() << "Order Insert Error:" << query.lastError().text();
        db.rollback();
        return false;
    }

    int orderId = query.lastInsertId().toInt();

    // 2. Child Items: UUID, Foreign Key, and Price
    for (const auto& item : m_items) {
        query.prepare("INSERT INTO order_items (id, order_id, menu_item_id, name, quantity, price_cents) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
        query.addBindValue(orderId);
        query.addBindValue(item.menu_item_id);
        query.addBindValue(item.name);
        query.addBindValue(item.quantity);
        query.addBindValue(static_cast<qlonglong>(item.price.cents));

        if (!query.exec()) {
            db.rollback();
            return false;
        }
    }

    if (db.commit()) {
        clearOrder(); // This wipes the memory list and updates UI
        return true;
    }
    return false;
}

void SalesModel::refresh() {
    if (m_currentOrderId == -1) return;

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

void SalesModel::loadOrderHistory() {
    beginResetModel();
    m_items.clear();

    QSqlQuery query("SELECT id, table_number, status, created_at FROM orders ORDER BY created_at DESC");
    while (query.next()) {
        OrderItem item;
        item.id = query.value("id").toString();
        item.name = "Order #" + item.id + " (Table " + query.value("table_number").toString() + ")";
        item.quantity = 1;
        // In a real app, you'd join with order_items to get the total or add a total_cents column to orders
        m_items.append(item);
    }
    endResetModel();
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

void SalesModel::viewOrderDetails(int orderId) {
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
    calculateTotal(); // Updates totalFormatted for the breakdown [cite: 19]
}

#include "historymodel.h"

HistoryModel::HistoryModel(QObject *parent) : QAbstractTableModel(parent) {}

int HistoryModel::rowCount(const QModelIndex &parent) const {
    return m_history.size();
}

int HistoryModel::columnCount(const QModelIndex &parent) const {
    return 3; // ID, Table, Date
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_history.size())
        return QVariant();

    const auto &record = m_history.at(index.row());

    switch (role) {
    case OrderIdRole:
        return record.id;
    case TableRole:
        return record.tableNumber;
    case DateRole:
        return record.createdAt;
    case DisplayTitleRole:
        return QString("Order #%1 (Table %2)").arg(record.id).arg(record.tableNumber);
    }
    return QVariant();
}

QHash<int, QByteArray> HistoryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[OrderIdRole] = "orderId";
    roles[TableRole] = "tableNumber";
    roles[DateRole] = "date";
    roles[DisplayTitleRole] = "displayTitle";
    return roles;
}

void HistoryModel::loadOrderHistory() {
    beginResetModel();
    m_history.clear();

    QSqlQuery query("SELECT id, table_number, created_at FROM orders ORDER BY id DESC");
    while (query.next()) {
        m_history.append({
            query.value(0).toInt(),
            query.value(1).toInt(),
            query.value(2).toString()
        });
    }
    endResetModel();
}
void HistoryModel::viewOrderDetails(int orderId) {
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

void HistoryModel::calculateTotal() {
    int64_t total = 0;
    for (const auto& item : m_items) {
        total += (item.price.cents * item.quantity);
    }
    m_totalMoney.cents = total;
    emit totalChanged();
}

QString HistoryModel::totalFormatted() const {
    return m_totalMoney.toString();
}



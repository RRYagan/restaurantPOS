#include "ordersview.h"

OrdersView::OrdersView(QObject *parent) : QAbstractTableModel(parent) {
    m_proxy = new UniversalFilterProxy(this);
    m_proxy->setSourceModel(this);
}

int OrdersView::rowCount(const QModelIndex &parent) const {
    return m_history.size();
}

int OrdersView::columnCount(const QModelIndex &parent) const {
    return 3; // ID, Table, Date
}

QVariant OrdersView::data(const QModelIndex &index, int role) const {
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

QHash<int, QByteArray> OrdersView::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[OrderIdRole] = "orderId";
    roles[TableRole] = "tableNumber";
    roles[DateRole] = "date";
    roles[DisplayTitleRole] = "displayTitle";
    return roles;
}

void OrdersView::loadOrders() {
    beginResetModel();
    m_history.clear();

    QSqlQuery query("SELECT id, table_number, created_at FROM orders ORDER BY id DESC");
    while (query.next()) {
        m_history.append({
            query.value(0).toString(),
            query.value(1).toInt(),
            query.value(2).toString()
        });
    }
    endResetModel();
}



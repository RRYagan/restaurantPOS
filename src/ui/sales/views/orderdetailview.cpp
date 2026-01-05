#include "orderdetailview.h"

OrderDetailView::OrderDetailView(QObject *parent) : QAbstractTableModel(parent) {}

int OrderDetailView::rowCount(const QModelIndex &parent) const {
    return m_items.size();
}

int OrderDetailView::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant OrderDetailView::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const auto &item = m_items.at(index.row());
    switch (role) {
    case NameRole: return item.name;
    case QuantityRole: return item.quantity;
    case PriceRole: return QVariant::fromValue(item.price);
    }
    return QVariant();
}

QHash<int, QByteArray> OrderDetailView::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[QuantityRole] = "quantity";
    roles[PriceRole] = "price";
    return roles;
}

void OrderDetailView::loadOrder(const QString& orderId) {
    beginResetModel();
    m_items.clear();

    QSqlQuery query;
    query.prepare("SELECT name, quantity, price_cents FROM order_items WHERE order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            Item item;
            item.name = query.value(0).toString();
            item.quantity = query.value(1).toInt();
            item.price.cents = query.value(2).toLongLong();
            m_items.append(item);
        }
    }
    endResetModel();
}

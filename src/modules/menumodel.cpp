#include "menumodel.h"
#include <QAbstractTableModel>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

MenuModel::MenuModel(QObject *parent) : QAbstractTableModel(parent) {
    refresh();
}

int MenuModel::rowCount(const QModelIndex &parent) const {
    return m_data.size();
}

int MenuModel::columnCount(const QModelIndex &parent) const {
    return 4; // id, name, price, icon
}

QVariant MenuModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_data.size())
        return QVariant();

    const MenuItem &item = m_data.at(index.row());

    switch (role) {
    case IdRole: return item.id;
    case NameRole: return item.name;
    case CategoryRole: return item.category;
    case PriceRole: return QVariant::fromValue(item.basePrice);
    case IconRole: return item.iconSource;
    case Qt::DisplayRole: return item.name; // Default display
    }
    return QVariant();
}

QHash<int, QByteArray> MenuModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[CategoryRole] = "category";
    roles[PriceRole] = "base_price_cents";
    roles[IconRole] = "icon_source";
    return roles;
}
void MenuModel::refresh() {
    beginResetModel();
    m_data.clear();

    // Use the global instance to ensure we are on the same connection
    QSqlQuery query("SELECT id, name, category, base_price_cents, icon_source FROM menu_items");

    while (query.next()) {
        MenuItem item;
        item.id = query.value(0).toInt();
        item.name = query.value(1).toString();
        item.category = query.value(2).toString();
        item.basePrice.cents = query.value(3).toLongLong();
        item.iconSource = query.value(4).toString();
        m_data.append(item);
    }
    endResetModel();
}

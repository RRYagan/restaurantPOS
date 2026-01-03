#include "databasemanager.h"
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

void MenuModel::setCurrentCategory(const QString &category) {
    if (m_currentCategory == category) return;

    m_currentCategory = category;
    refresh(); // This now does the heavy lifting via DB Manager
    emit currentCategoryChanged();
}

void MenuModel::refresh() {
    beginResetModel();
    // Ask the DB Manager for the specific data we need right now
    m_data = DatabaseManager::instance().fetchMenuItems(m_currentCategory);
    endResetModel();
}
// In menumodel.cpp

bool MenuModel::addMenuItem(const QString &name, const QString &category, int price, const QString &icon) {
    if (DatabaseManager::instance().addMenuItem(name, category, price, icon)) {
        refresh(); // Refresh UI list
        return true;
    }
    return false;
}


bool MenuModel::deleteItem(int itemId) {
    if (DatabaseManager::instance().deleteMenuItem(itemId)) {
        refresh();
        return true;
    }
    return false;
}

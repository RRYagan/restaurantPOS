#include "categorymodel.h"

CategoryModel::CategoryModel(QObject *parent) : QAbstractListModel(parent) {
    refresh();
}

int CategoryModel::rowCount(const QModelIndex &parent) const {
    return m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_categories.size()) return QVariant();
    return m_categories.at(index.row());
}

QHash<int, QByteArray> CategoryModel::roleNames() const {
    return { {NameRole, "name"} };
}

void CategoryModel::refresh() {
    beginResetModel();
    // Logic moved to DB Manager
    m_categories = DatabaseManager::instance().fetchCategories();
    endResetModel();
}

bool CategoryModel::addCategory(const QString &name) {
    if (DatabaseManager::instance().addCategory(name)) {
        refresh(); // Refresh locally
        return true;
    }
    return false;
}


bool CategoryModel::editCategory(const QString &oldName, const QString &newName) {
    if (newName.trimmed().isEmpty() || oldName == newName) return false;
    if (DatabaseManager::instance().updateCategory(oldName, newName)) {
        refresh();
        return true;
    }
    return false;
}

bool CategoryModel::deleteCategory(const QString &name) {
    if (name == "All") return false; // Protected system default
    if (DatabaseManager::instance().deleteCategory(name)) {
        refresh();
        return true;
    }
    return false;
}

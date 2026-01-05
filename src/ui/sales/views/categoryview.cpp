#include "categoryview.h"

CategoryView::CategoryView(QObject *parent) : QAbstractListModel(parent) {
    refresh();
}

int CategoryView::rowCount(const QModelIndex &parent) const {
    return m_categories.size();
}

QVariant CategoryView::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_categories.size()) return QVariant();
    return m_categories.at(index.row());
}

QHash<int, QByteArray> CategoryView::roleNames() const {
    return { {NameRole, "name"} };
}

void CategoryView::refresh() {
    beginResetModel();
    m_categories.clear();
    // Logic moved to DB Manager
    m_categories = DatabaseManager::instance().fetchCategories();
    endResetModel();
}

bool CategoryView::addCategory(const QString &name) {
    if (DatabaseManager::instance().addCategory(name)) {
        refresh(); // Refresh locally
        return true;
    }
    return false;
}


bool CategoryView::editCategory(const QString &oldName, const QString &newName) {
    if (newName.trimmed().isEmpty() || oldName == newName) return false;
    if (DatabaseManager::instance().updateCategory(oldName, newName)) {
        refresh();
        return true;
    }
    return false;
}

bool CategoryView::deleteCategory(const QString &name) {
    if (name == "All") return false; // Protected system default
    if (DatabaseManager::instance().deleteCategory(name)) {
        refresh();
        return true;
    }
    return false;
}

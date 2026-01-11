#ifndef MENUCATEGORYMODEL_H
#define MENUCATEGORYMODEL_H

#include <QObject>
#include <QStringList>

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "menutablemodel.h"

// Category model for QML
class MenuCategoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, NameRole };
    explicit MenuCategoryModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    void setCategories(const QVector<MenuCategory>& categories) {
        beginResetModel();
        m_categories = categories;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = {}) const override { Q_UNUSED(parent); return m_categories.size(); }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_categories.size()) return {};
        const auto &c = m_categories[index.row()];
        if (role == IdRole) return c.id;
        if (role == NameRole) return c.name;
        return {};
    }

    QHash<int, QByteArray> roleNames() const override { return { {IdRole, "id"}, {NameRole, "name"} }; }

private:
    QVector<MenuCategory> m_categories;
};

#endif

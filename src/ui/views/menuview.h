#ifndef MENUMODEL_H
#define MENUMODEL_H

#include <QAbstractTableModel>
#include <basemodel.h>
#include <databasemanager.h>
#include <universalfilterproxy.h>
#include "menuitem.h"
#include <QtQml/qqmlregistration.h>

class MenuView : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)

public:
    enum MenuRole { IdRole = Qt::UserRole + 1, NameRole, CategoryRole, PriceRole, IconRole };

    explicit MenuView(QObject *parent = nullptr);

    UniversalFilterProxy* proxy() const { return m_proxy; }

    QStringList categories() const {
        return DatabaseManager::instance().fetchCategories(); // Returns list from DB
    }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addMenuItem(const QString &name, const QString &category, int price, const QString &icon);
    Q_INVOKABLE bool deleteItem(int itemId);
signals:
    void categoriesChanged();

private:
    BaseModel *m_sourceModel;
    UniversalFilterProxy *m_proxy;
};

#endif

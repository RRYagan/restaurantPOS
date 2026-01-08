#ifndef MENUVIEW_H
#define MENUVIEW_H

#include <QAbstractTableModel>
#include <basemodel.h>
#include <menumodel.h>
#include <universalfilterproxy.h>
#include "menuitem.h"
#include "categorymodel.h"
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

    // QStringList categories() const {
    //     // MenuModel menuModel;
        // return m_model.getAllCategories();
    // }

    QVariantList getAllMenuItems() const;
    QStringList categories() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addMenuItem(const QString &name, const QString &category, int price, const QString &icon);
    Q_INVOKABLE bool deleteItem(int itemId);
    Q_INVOKABLE bool updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon);
signals:
    void categoriesChanged();

private:
    BaseModel *m_sourceModel;
    UniversalFilterProxy *m_proxy;
    MenuModel m_model;
};

#endif

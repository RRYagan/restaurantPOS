#ifndef MENUMODEL_H
#define MENUMODEL_H

#pragma once
#include <QObject>
#include <QAbstractTableModel>
#include <QtQml/qqmlregistration.h>
#include "menuitem.h"

class MenuModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum MenuRole {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CategoryRole,
        PriceRole,
        IconRole
    };

    explicit MenuModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
private:
    QVector<MenuItem> m_data;
};

#endif // MENUMODEL_H

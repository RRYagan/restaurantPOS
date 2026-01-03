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

    Q_PROPERTY(QString currentCategory READ currentCategory WRITE setCurrentCategory NOTIFY currentCategoryChanged)

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
    QString currentCategory() const { return m_currentCategory; }
    void setCurrentCategory(const QString &category);

signals:
    void currentCategoryChanged();

private:
    QVector<MenuItem> m_data;
    QVector<MenuItem> m_allItems;
    QString m_currentCategory = "All";
};

#endif // MENUMODEL_H

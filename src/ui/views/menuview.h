#ifndef MENUMODEL_H
#define MENUMODEL_H

#include <QAbstractTableModel>
#include "menuitem.h"
#include <QtQml/qqmlregistration.h>

class MenuView : public QAbstractTableModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString currentCategory READ currentCategory WRITE setCurrentCategory NOTIFY currentCategoryChanged)

public:
    enum MenuRole { IdRole = Qt::UserRole + 1, NameRole, CategoryRole, PriceRole, IconRole };

    explicit MenuView(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentCategory() const { return m_currentCategory; }
    void setCurrentCategory(const QString &category);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool addMenuItem(const QString &name, const QString &category, int price, const QString &icon);
    Q_INVOKABLE bool deleteItem(int itemId);

signals:
    void currentCategoryChanged();

private:
    QVector<MenuItem> m_data;
    QString m_currentCategory = "All";
};

#endif

#ifndef INVENTORYMODEL_H
#define INVENTORYMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <inventoryitem.h>
#include <QtQml/qqmlregistration.h>

class InventoryView : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
public:
    enum InventoryRoles { IdRole = Qt::UserRole + 1, NameRole, QuantityRole, UnitRole };
    explicit InventoryView(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(); // Pulls latest from DB
    Q_INVOKABLE bool addStock(const QString &name, int qty, const QString &unit);
    Q_INVOKABLE bool updateStock(int id, const QString &name, int qty, const QString &unit);

private:
    QVector<InventoryItem> m_items;
};

#endif

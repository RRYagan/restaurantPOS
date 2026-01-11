#ifndef ORDERDETAILVIEW_H
#define ORDERDETAILVIEW_H

#include <QAbstractTableModel>
#include <QSqlQuery>
#include <QtQml/qqmlregistration.h>
#include "money.h"

class OrderDetailView : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum OrderDetailRoles {
        NameRole = Qt::UserRole + 1,
        QuantityRole,
        PriceRole
    };

    explicit OrderDetailView(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // The only way to populate this view is by providing an existing Order ID
    Q_INVOKABLE void loadOrder(const QString& orderId);

private:
    struct Item {
        QString name;
        int quantity;
        Money price;
    };
    QList<Item> m_items;
};

#endif

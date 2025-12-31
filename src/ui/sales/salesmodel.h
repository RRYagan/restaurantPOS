#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QAbstractTableModel>
#include <QtQml/qqmlregistration.h>
#include "money.h"

// Structure to represent an item in the current active order
struct OrderItem {
    QString id;
    int menu_item_id;
    int quantity;
    QString name;

    Money price;
};

class SalesModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    // Property for QML to display the running total of the current order [cite: 19]
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY totalChanged)

public:
    enum SalesRoles {
        QuantityRole = Qt::UserRole + 1,
        NameRole,
        PriceRole,
        ItemIdRole,
        MenuIdRole
    };

    explicit SalesModel(QObject *parent = nullptr);

    // Table Model overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Logic to create a new order and add the first item
    Q_INVOKABLE bool startNewOrder(int menuItemId);

    // Refresh the model from the database
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addItemToOrder(int menuItemId);
    Q_INVOKABLE bool makeOrder();
    Q_INVOKABLE void clearOrder(); // To reset for a new customer
    QString totalFormatted() const;

signals:
    void totalChanged();

private:
    QList<OrderItem> m_items;
    int m_currentOrderId = -1;
    void calculateTotal();
    Money m_totalMoney;
};

#endif // SALESMODEL_H

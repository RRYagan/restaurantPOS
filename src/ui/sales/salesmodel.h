#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QAbstractTableModel>
#include <orderitem.h>
#include <QtQml/qqmlregistration.h>
#include "money.h"

class SalesModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
    // Property for QML to display the running total of the current order
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY totalChanged)
    Q_PROPERTY(QString currentOrderId READ currentOrderId NOTIFY currentOrderIdChanged)

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

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addItemToOrder(int menuItemId);
    Q_INVOKABLE bool makeOrder();
    Q_INVOKABLE void clearOrder(); // To reset for a new customer
    // Q_INVOKABLE void loadOrderHistory();
    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void updateQuantity(int index, int newQuantity);
    Q_INVOKABLE void viewOrderDetails(const QString& orderId);
    Q_INVOKABLE void switchToCart();
    QString totalFormatted() const;
    QString currentOrderId() const { return m_currentOrderId; }

signals:
    void totalChanged();
    void currentOrderIdChanged();

private:
    QList<OrderItem> m_items;
    QList<OrderItem> hd_items;
    QString m_currentOrderId = "";
    QList<OrderItem> m_activeCart; // Keep the actual cart safe here
    bool m_isShowingHistory = false;
    void calculateTotal();
    Money m_totalMoney;

};

#endif // SALESMODEL_H

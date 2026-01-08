#ifndef SALESVIEW_H
#define SALESVIEW_H

#include <QAbstractTableModel>
#include <basemodel.h>
#include <orderitem.h>
#include <ordermodel.h>
#include <universalfilterproxy.h>
#include <QtQml/qqmlregistration.h>
#include "money.h"

class SalesView : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY totalChanged)
    Q_PROPERTY(QString currentOrderId READ currentOrderId NOTIFY currentOrderIdChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)

public:
    explicit SalesView(QObject *parent = nullptr);

    UniversalFilterProxy* proxy() const { return m_proxy; }

    // Q_INVOKABLE void refresh();
    Q_INVOKABLE void addItemToOrder(int menuItemId);
    Q_INVOKABLE bool makeOrder();
    Q_INVOKABLE void clearOrder(); // To reset for a new customer
    Q_INVOKABLE void removeItem(int proxyIndex);
    Q_INVOKABLE void updateQuantity(int proxyIndex, int newQuantity);

    QString totalFormatted() const;
    QString currentOrderId() const { return m_currentOrderId; }
    bool isBusy() const { return m_isBusy; }

signals:
    void totalChanged();
    void currentOrderIdChanged();
    void isBusyChanged();

private:
    QVariantList getSalesData(); // Provider for BaseModel

    QList<OrderItem> m_items;
    BaseModel *m_internalModel;
    UniversalFilterProxy *m_proxy;
    bool m_isBusy = false;
    void calculateTotal();

    QString m_currentOrderId = "";
    Money m_totalMoney;
    OrderModel m_model;

};

#endif // SALESMODEL_H

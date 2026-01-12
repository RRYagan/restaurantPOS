#ifndef ORDERVIEWCONTROLLER_H
#define ORDERVIEWCONTROLLER_H

#include <QObject>
#include "ordertablemodel.h"
#include "databasemanager.h"
#include <QtQml/qqmlregistration.h>

class OrderViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(OrderTableModel* orderModel READ orderModel CONSTANT)

public:
    explicit OrderViewController(QObject *parent = nullptr);

    OrderTableModel* orderModel() const { return m_orderModel; }

    Q_INVOKABLE void createNewOrder(int tableNum, int amount);
    Q_INVOKABLE void markAsPaid(const QString &orderId);
    Q_INVOKABLE void refresh();

private:
    OrderTableModel *m_orderModel;
};

#endif

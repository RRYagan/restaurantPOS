#ifndef ORDERVIEWCONTROLLER_H
#define ORDERVIEWCONTROLLER_H

#include <QObject>
#include <QString>
#include <QLocale>
#include "ordertablemodel.h"
#include <QtQml/qqmlregistration.h>


class OrderViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    // Access to the list of items for the Cart ListView
    Q_PROPERTY(OrderTableModel* model READ model CONSTANT)
    // Display properties for the UI
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY orderChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY orderChanged)

public:
    explicit OrderViewController(QObject* parent = nullptr);

    OrderTableModel* model() const { return m_model; }

    // UI Helpers
    QString totalFormatted() const;
    int itemCount() const;

    // Logic
    Q_INVOKABLE void addItem(const QVariantMap& itemData);
    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void clearOrder();
    Q_INVOKABLE bool makeOrder(); // Finalize and save to DB
    // Internal C++ version if needed
    void addItem(const OrderItem& item);
    Q_INVOKABLE void updateQuantity(int index, double newQuantity);
    void calculateTotal();
    int currentTotalCents() const { return m_currentTotalCents; }


signals:
    void orderChanged();
    void totalsChanged();

private:
    QList<OrderItem> m_items;
    bool m_isBusy = false;

    OrderTableModel* m_model;
    QLocale m_locale;

    // QVector<Order> m_orders;
    QVector<OrderItem> m_currentItems; // Items in the "Cart"
    int64_t m_currentTotalCents = 0;
};

#endif

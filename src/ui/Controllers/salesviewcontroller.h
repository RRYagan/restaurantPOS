#ifndef SALESVIEWCONTROLLER_H
#define SALESVIEWCONTROLLER_H

#include <QObject>
#include <inventorymodel.h>
#include "salesmodel.h"
#include "productmodel.h"
#include "inventoryviewcontroller.h"
#include <QtQml/qqmlregistration.h>


class SalesViewController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SalesModel* orderModel READ orderModel CONSTANT)
    Q_PROPERTY(ProductModel* productModel READ productModel CONSTANT)

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(double totalAmount READ totalAmount NOTIFY orderChanged)
    Q_PROPERTY(double totalTaxFormatted READ totalTaxFormatted NOTIFY orderChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    explicit SalesViewController(QObject* parent = nullptr);

    [[nodiscard]] auto orderModel() const -> SalesModel* { return m_salesModel; }
    [[nodiscard]] auto productModel() const -> ProductModel* { return m_productModel; }

    [[nodiscard]] auto isBusy() const -> bool { return m_isBusy; }
    [[nodiscard]] auto totalAmount() const -> double { return m_salesModel->totalAmount(); }
    [[nodiscard]] auto totalTaxFormatted() const -> double { return m_salesModel->totalTaxAmount(); }
    [[nodiscard]] auto itemCount() const -> int {
        return m_salesModel->count();
    }

    // --- MAINTAINED Q_INVOKABLES ---
    Q_INVOKABLE bool addItem(const QString& productId);
    Q_INVOKABLE bool removeItem(int index);
    Q_INVOKABLE bool updateQuantity(int index, double qty);
    Q_INVOKABLE void clearOrder();
    Q_INVOKABLE bool makeOrder();
    Q_INVOKABLE bool editOrder(const QString& orderId);

signals:
    void orderChanged();
    void itemCountChanged();
    void isBusyChanged();

private:
    SalesModel* m_salesModel;
    ProductModel* m_productModel;
    InventoryViewController* m_inventoryController;
    bool m_isBusy = false;
};
#endif

#ifndef SALESVIEWCONTROLLER_H
#define SALESVIEWCONTROLLER_H

#include <QObject>
#include <memory> // For std::unique_ptr
// #include <orderitemmodel.h>
#include <genericfilterproxymodel.h>
#include <QtQml/qqmlregistration.h>
#include "salesmodel.h"
#include "productmodel.h"
#include "inventoryviewcontroller.h"



class SalesViewController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // Q_PROPERTY: Still returns raw pointers for QML compatibility
    Q_PROPERTY(SalesModel* orderModel READ orderModel CONSTANT)
    // Q_PROPERTY(OrderItemModel* orderItemModel READ orderItemModel CONSTANT)
    Q_PROPERTY(ProductModel* productModel READ productModel CONSTANT)
    Q_PROPERTY(GenericFilterProxyModel* filteredProducts READ filteredProducts CONSTANT)
    // Q_PROPERTY(GenericFilterProxyModel* kitchenProxy READ kitchenProxy CONSTANT)

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(double totalAmount READ totalAmount NOTIFY orderChanged)
    Q_PROPERTY(double totalTaxFormatted READ totalTaxFormatted NOTIFY orderChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    explicit SalesViewController(QObject* parent = nullptr);

    // Explicitly mark getters as nodiscard and const to express intent (P.3)
    [[nodiscard]] auto orderModel() const -> SalesModel* { return m_salesModel.get(); }
    [[nodiscard]] auto orderItemModel() const -> OrderItemModel* { return m_orderItemModel.get(); }
    [[nodiscard]] auto productModel() const -> ProductModel* { return m_productModel.get(); }
    GenericFilterProxyModel* filteredProducts() const { return m_genericFilterProxyModel.get(); }
    // GenericFilterProxyModel* kitchenProxy() const { return m_genericFilterProxyModel.get(); }


    [[nodiscard]] auto isBusy() const -> bool { return m_isBusy; }
    [[nodiscard]] auto totalAmount() const -> double;
    [[nodiscard]] auto totalTaxFormatted() const -> double;
    [[nodiscard]] auto itemCount() const -> int;

    // Q_INVOKABLES for QML
    Q_INVOKABLE bool addItem(const QString& productId);
    Q_INVOKABLE bool removeItem(int index);
    Q_INVOKABLE bool updateQuantity(int index, double qty);
    Q_INVOKABLE void clearOrder();
    Q_INVOKABLE bool makeOrder();
    Q_INVOKABLE bool editOrder(const QString& orderId);

    [[nodiscard]] Q_INVOKABLE QVariantList kitchenOrders(const QString& serviceState) const;
    Q_INVOKABLE void updateAllStatus(const QString &orderId, const QString &status);
    Q_INVOKABLE void updateItemStatus(const QString &itemId, const QString &status);
    [[nodiscard]] Q_INVOKABLE QVariantList loadOrders() const;
    Q_INVOKABLE QVariant createItemProxy(const QVariant& rawItems, const QString& status);

signals:
    void orderChanged();
    void itemCountChanged();
    void isBusyChanged();
    void kitchenDataChanged();

private:
    // R.20: Use unique_ptr to manage ownership and prevent leaks
    std::unique_ptr<SalesModel> m_salesModel;
    std::unique_ptr<OrderItemModel> m_orderItemModel;
    std::unique_ptr<ProductModel> m_productModel;
    std::unique_ptr<InventoryViewController> m_inventoryController;
    std::unique_ptr<GenericFilterProxyModel> m_genericFilterProxyModel;

    // C.48: Prefer default member initializers
    bool m_isBusy{false};
};

#endif // SALESVIEWCONTROLLER_H

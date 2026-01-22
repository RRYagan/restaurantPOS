// #ifndef SALESVIEWCONTROLLER_H
// #define SALESVIEWCONTROLLER_H

// #include <QObject>
// #include <QVariantMap>
// #include <QString>
// #include "ordermodel.h"
// #include "orderitemmodel.h"
// // #include "product.h"
// #include <QtQml/qqmlregistration.h>


// class SalesViewController : public QObject {
//     Q_OBJECT
//     QML_ELEMENT

//     Q_PROPERTY(OrderModel* orderModel READ orderModel CONSTANT)
//     Q_PROPERTY(OrderItemModel* itemModel READ itemModel CONSTANT)
//     Q_PROPERTY(ProductModel* productModel READ productModel CONSTANT)
//     Q_PROPERTY(QString currentOrderId READ currentOrderId NOTIFY orderIdChanged)
//     Q_PROPERTY(double totalAmount READ totalAmount NOTIFY orderChanged)
//     Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY orderChanged)
//     Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
//     Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
//     Q_PROPERTY(QString totalTaxFormatted READ totalTaxFormatted NOTIFY orderChanged)

// public:
//     explicit SalesViewController(QObject* parent = nullptr);

//     /* Getters for QML properties */
//     [[nodiscard]] auto orderModel() const -> OrderModel* { return m_orderModel; }
//     [[nodiscard]] auto itemModel() const -> OrderItemModel* { return m_itemModel; }
//     [[nodiscard]] auto productModel() const -> ProductModel* { return m_productModel; }

//     [[nodiscard]] auto currentOrderId() const -> QString { return m_currentOrderId; }
//     [[nodiscard]] auto isBusy() const -> bool { return m_isBusy; }
//     [[nodiscard]] auto totalAmount() const -> double {
//         /* Safety check: returns 0.0 if the model is null */
//         return m_itemModel ? m_itemModel->totalAmount().toKSH() : 0.0;
//     }
//     [[nodiscard]] auto totalFormatted() const -> QString {
//         return QString::number(totalAmount(), 'f', 2);
//     }
//     [[nodiscard]] auto totalTaxAmount() const -> double {
//         return m_itemModel ? m_itemModel->totalTaxAmount().toKSH() : 0.0;
//     }
//     [[nodiscard]] auto totalTaxFormatted() const -> QString {
//         return QString::number(totalTaxAmount(), 'f', 2);
//     }
//     [[nodiscard]] auto itemCount() const -> int {
//         return m_itemModel ? m_itemModel->count() : 0;
//     }

//     /* QML Invokables: Trailing returns, [[nodiscard]] and auto not supported by moc */
//     Q_INVOKABLE bool addItem(const QString& productId);
//     Q_INVOKABLE bool removeItem(int index);
//     Q_INVOKABLE bool updateQuantity(int index, double qty);
//     Q_INVOKABLE void clearOrder();
//     /* Transaction Management */
//     Q_INVOKABLE bool makeOrder();
//     // Q_INVOKABLE bool finalizeSale(const QVariantMap& paymentData);

// signals:
//     void orderChanged();
//     void itemCountChanged();
//     void orderIdChanged();
//     void isBusyChanged();

// private:
//     auto mapToProduct(const QVariantMap& d) -> Product;
//     bool m_isBusy = false;
//     OrderModel* m_orderModel = nullptr;
//     OrderItemModel* m_itemModel = nullptr;
//     ProductModel* m_productModel = nullptr;
//     QString m_currentOrderId;
// };

// #endif

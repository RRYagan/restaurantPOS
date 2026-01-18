#ifndef SALESVIEWCONTROLLER_H
#define SALESVIEWCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <productmodel.h>
#include "ordermodel.h"
#include "orderitemmodel.h"
#include <QtQml/qqmlregistration.h>

struct StagedItem {
    Product product;
    double quantity;
    double finalUnitPrice; // base price + modifiers
    QString modifiersJson;
};

class SalesViewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Properties used by SalesScreen.qml
    Q_PROPERTY(OrderModel* orderModel READ orderModel CONSTANT)
    Q_PROPERTY(OrderItemModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY orderChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY orderChanged)
    Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy NOTIFY isBusyChanged)

public:
    explicit SalesViewController(QObject* parent = nullptr);

    ~SalesViewController() override = default;
    SalesViewController(const SalesViewController&) = delete;
    auto operator=(const SalesViewController&) -> SalesViewController& = delete;
    SalesViewController(SalesViewController&&) = delete;
    auto operator=(SalesViewController&&) -> SalesViewController& = delete;

    // order staging
    [[nodiscard]] auto totalFormatted() const -> QString;
    [[nodiscard]] auto itemCount() const -> int { return static_cast<int>(m_stagedItems.size()); }

    // Getters using modern trailing return types
    [[nodiscard]] auto orderModel() const -> OrderModel* { return m_orderModel; }
    [[nodiscard]] auto itemModel() const -> OrderItemModel* { return m_itemModel; }
    // [[nodiscard]] auto totalFormatted() const -> QString;
    // [[nodiscard]] auto itemCount() const -> int;
    [[nodiscard]] auto isBusy() const -> bool { return m_isBusy; }

    // Property Setter
    auto setIsBusy(bool busy) -> void;

    // QML Invokable Actions
    // Using QVariantMap to solve "easily-swappable-parameters" while keeping C++ < 11
    Q_INVOKABLE bool addItem(const QVariantMap &data);
    Q_INVOKABLE bool updateQuantity(int index, double newQuantity);
    Q_INVOKABLE bool removeItem(int index);
    Q_INVOKABLE void clearOrder();
    Q_INVOKABLE bool makeOrder();

    Q_INVOKABLE void addWithModifiers(const QVariantMap& itemData, const QVariantList& modifiers);

    /*TODO: move logic to salesmodel*/
    Q_INVOKABLE bool finalizeSale(const QVariantMap& paymentData);
signals:
    void orderChanged();
    void isBusyChanged();

private:
    OrderModel* m_orderModel = nullptr;
    OrderItemModel* m_itemModel = nullptr;
    bool m_isBusy = false;

    auto mapToProduct(const QVariantMap& d) -> Product;
    QList<StagedItem> m_stagedItems;
    QString m_currentOrderId;
};

#endif // SALESVIEWCONTROLLER_H

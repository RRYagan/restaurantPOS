#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <cstdint>

struct OrderItem {
    QString id;
    QString orderId;
    QString productId;
    double quantity;
    double unitPrice;
    QString serviceState;
    /* Helper calculation */
    [[nodiscard]] auto total() const -> double { return quantity * unitPrice; }
};


struct ItemId { QString value; };
enum class ServiceStateValue : std::uint8_t { Ordered, Preparing, Served };

struct ServiceState {
    ServiceStateValue value;
    [[nodiscard]] auto toString() const -> QString {
        switch (value) {
        case ServiceStateValue::Ordered:   return "ordered";
        case ServiceStateValue::Preparing: return "preparing";
        case ServiceStateValue::Served:    return "served";
        }
        return "ordered";
    }
};

class OrderItemModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit OrderItemModel(QObject* parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    ~OrderItemModel() override = default;
    OrderItemModel(const OrderItemModel&) = delete;
    auto operator=(const OrderItemModel&) -> OrderItemModel& = delete;
    OrderItemModel(OrderItemModel&&) = delete;
    auto operator=(OrderItemModel&&) -> OrderItemModel& = delete;

    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        OrderIdRole,
        ProductIdRole,
        QuantityRole,
        UnitPriceRole,
        ServiceStateRole,
        RowTotalRole /* Computed role for UI convenience */
    };

    // --- CONFIGURATION ---
    // Filters the model to show items only for this specific order
    auto setOrderId(const QString& orderId) -> void;

    /* --- READ --- */
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    /* --- WRITE --- */
    auto setData(const QModelIndex& index, const QVariant& value, int role) -> bool override;

    /* --- CRUD --- */
    /* Expects: { "order_id": str, "product_id": str, "quantity": real, "unit_price": real } */
    auto addOrderItem(const QVariantMap &data) -> bool;
    auto updateOrderItem(const QVariantMap &data) -> bool;
    auto removeItem(const QString& itemId) -> bool;

    /* --- HELPERS --- */
    [[nodiscard]] auto calculateOrderTotal() const -> double;

private:
    QString m_currentOrderId;
    int m_idCol = -1;
    int m_orderIdCol = -1;
    int m_productIdCol = -1;
    int m_qtyCol = -1;
    int m_priceCol = -1;
    int m_stateCol = -1;
};

#endif // ORDERITEMMODEL_H

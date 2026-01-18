#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include "ordermodel.h"

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
    auto total() const -> double { return quantity * unitPrice; }
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
    /* Returns true if successful */
    auto addItem(const QString& orderId, const QString& productId, double quantity, double unitPrice) -> bool;

    auto updateQuantity(const QString& itemId, double quantity) -> bool;
    auto updateServiceState(const OrderId& itemId, const ServiceState& state) -> bool;
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

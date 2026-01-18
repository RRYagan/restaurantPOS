#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVariantMap>
#include <cstdint>
#include <QDateTime>
#include <cstdint>

struct Order {
    QString id;
    QString tableNumber;
    QString waiterId;
    QString orderStatus;
    QDateTime createdAt;
};

// ordermodel.h
struct OrderId {
    QString value;
    explicit OrderId(QString v) : value(std::move(v)) {}
};

enum class OrderStatusValue: std::uint8_t {
    Open,
    Closed,
    Voided
};

enum class ServiceStateValue : std::uint8_t {
    Ordered,
    Preparing,
    Served
};

struct OrderStatus {
    OrderStatusValue value;

    /* Helper to convert Enum to String for Database */
    [[nodiscard]] auto toString() const -> QString {
        switch (value) {
        case OrderStatusValue::Open:   return "open";
        case OrderStatusValue::Closed: return "closed";
        case OrderStatusValue::Voided: return "voided";
        }
        return "open";
    }

    /* Helper to convert String from Database back to Enum */
    static auto fromString(const QString& str) -> OrderStatusValue {
        if (str == "closed") return OrderStatusValue::Closed;
        if (str == "voided") return OrderStatusValue::Voided;
        return OrderStatusValue::Open;
    }
};

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

    static auto fromString(const QString& str) -> ServiceStateValue {
        if (str == "preparing") return ServiceStateValue::Preparing;
        if (str == "served")    return ServiceStateValue::Served;
        return ServiceStateValue::Ordered;
    }
};

struct TableNumber {
    QString value;
    explicit TableNumber(QString v) : value(std::move(v)) {}
};

struct WaiterId {
    QString value;
    explicit WaiterId(QString v) : value(std::move(v)) {}
};


class OrderModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit OrderModel(QObject* parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    ~OrderModel() override = default;
    OrderModel(const OrderModel&) = delete;
    auto operator=(const OrderModel&) -> OrderModel& = delete;
    OrderModel(OrderModel&&) = delete;
    auto operator=(OrderModel&&) -> OrderModel& = delete;

    enum Roles : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        TableNumberRole,
        WaiterIdRole,
        StatusRole,
        CreatedAtRole
    };

    // --- READ ---
    [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    // --- WRITE ---
    auto setData(const QModelIndex& index, const QVariant& value, int role) -> bool override;

    // --- CRUD ---
    // Creates a new "open" order. Returns the new Order ID.
    auto createOrder(const TableNumber& table, const WaiterId& waiter) -> QString;
    // Updates status (e.g., to "closed" or "voided")
    auto updateOrderStatus(const OrderId& orderId,const OrderStatus& status) -> bool;
    auto removeOrder(const OrderId& orderId) -> bool;
    [[nodiscard]] auto orderAt(int row) const -> Order;

private:
    int m_idCol = -1;
    int m_tableCol = -1;
    int m_waiterCol = -1;
    int m_statusCol = -1;
    int m_createdCol = -1;
};

#endif // ORDERMODEL_H

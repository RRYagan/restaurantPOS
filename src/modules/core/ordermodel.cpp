#include "ordermodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QDebug>

OrderModel::OrderModel(QObject* parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
    setTable("customer_order");
    setEditStrategy(OnManualSubmit);

    // Cache indices
    m_idCol      = fieldIndex("id");
    m_tableCol   = fieldIndex("table_number");
    m_waiterCol  = fieldIndex("waiter_id");
    m_statusCol  = fieldIndex("order_status");
    m_createdCol = fieldIndex("created_at");

    if (!select()) {
        qCritical() << "OrderModel select failed:" << lastError().text();
    }
}

auto OrderModel::roleNames() const -> QHash<int, QByteArray>
{
    return {
        { IdRole, "id" },
        { TableNumberRole, "tableNumber" },
        { WaiterIdRole, "waiterId" },
        { StatusRole, "orderStatus" },
        { CreatedAtRole, "createdAt" }
    };
}

[[nodiscard]] auto OrderModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid()) return {};
    if (role < Qt::UserRole) return QSqlTableModel::data(index, role);

    const int row = index.row();
    switch (role) {
    case IdRole:          return QSqlTableModel::data(this->index(row, m_idCol));
    case TableNumberRole: return QSqlTableModel::data(this->index(row, m_tableCol));
    case WaiterIdRole:    return QSqlTableModel::data(this->index(row, m_waiterCol));
    case StatusRole:      return QSqlTableModel::data(this->index(row, m_statusCol));
    case CreatedAtRole:   return QSqlTableModel::data(this->index(row, m_createdCol));
    default: return {};
    }
}

auto OrderModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    if (!index.isValid()) return false;
    QSqlRecord rec = record(index.row());

    switch (role) {
    case TableNumberRole: rec.setValue(m_tableCol, value); break;
    case WaiterIdRole:    rec.setValue(m_waiterCol, value); break;
    case StatusRole:      rec.setValue(m_statusCol, value); break;
    default: return false;
    }

    if (setRecord(index.row(), rec)) {
        emit dataChanged(index, index, {role});
        return submitAll(); // Auto-save on direct set
    }
    return false;
}

auto OrderModel::createOrder(const TableNumber& table, const WaiterId& waiter) -> QString
{
    QSqlQuery query(database());
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare(R"(
        INSERT INTO customer_order (id, table_number, waiter_id, order_status, created_at)
        VALUES (:id, :table, :waiter, 'open', CURRENT_TIMESTAMP)
    )");

    query.bindValue(":id", newId);
    query.bindValue(":table", table.value); // Use .value
    query.bindValue(":waiter", waiter.value); // Use .value

    if (!query.exec()) {
        qCritical() << "Failed to create order:" << query.lastError().text();
        return {};
    }

    select();
    return newId;
}


auto OrderModel::updateOrderStatus(const OrderId& orderId, const OrderStatus& status) -> bool
{
    QSqlQuery query(database());
    query.prepare("UPDATE customer_order SET order_status = :status WHERE id = :id");
    query.bindValue(":status", status.toString());
    query.bindValue(":id", orderId.value);

    if (!query.exec()) {
        qCritical() << "Failed update status:" << query.lastError().text();
        return false;
    }
    return select();
}

auto OrderModel::removeOrder(const OrderId& orderId) -> bool
{
    QSqlQuery query(database());
    query.prepare("DELETE FROM customer_order WHERE id = :id");
    query.bindValue(":id", orderId.value);

    if (!query.exec()) {
        qCritical() << "Failed delete order:" << query.lastError().text();
        return false;
    }
    return select();
}


[[nodiscard]] auto OrderModel::orderAt(int row) const -> Order
{
    if (row < 0 || row >= rowCount()) return {};
    QSqlRecord rec = record(row);

    return Order {
        rec.value(m_idCol).toString(),
        rec.value(m_tableCol).toString(),
        rec.value(m_waiterCol).toString(),
        rec.value(m_statusCol).toString(),
        rec.value(m_createdCol).toDateTime()
    };
}

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

auto OrderModel::addOrder(const QVariantMap &data) -> QString {
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery query(database());
    query.prepare("INSERT INTO customer_order (id, table_number, waiter_id) VALUES (:id, :t, :w)");
    query.bindValue(":id", newId);
    query.bindValue(":t", data.value("table_number").toString());
    query.bindValue(":w", data.value("waiter_id").toString());

    if (!query.exec()) {
        qCritical() << "Add Order Error:" << query.lastError().text();
        return {};
    }
    select();
    return newId;
}

auto OrderModel::updateOrder(const QVariantMap &data) -> bool {
    QString id = data.value("id").toString();
    if (id.isEmpty()) return false;

    QSqlQuery query(database());
    // Flexible update: allows updating table, waiter, or status via one map
    QStringList updates;
    if (data.contains("table_number")) {
        updates << "table_number = :t";
    }
    if (data.contains("waiter_id")){
        updates << "waiter_id = :w";
    }
    if (data.contains("order_status")){
        updates << "order_status = :s";
    }

    if (updates.isEmpty()){
        return false;
    }

    query.prepare(QString("UPDATE customer_order SET %1 WHERE id = :id").arg(updates.join(", ")));
    query.bindValue(":id", id);
    if (data.contains("table_number")){
        query.bindValue(":t", data.value("table_number").toString());
    }
    if (data.contains("waiter_id")){
        query.bindValue(":w", data.value("waiter_id").toString());

    }
    if (data.contains("order_status")) {
        /* Handle both raw string or enum-based status */
        query.bindValue(":s", data.value("order_status").toString());
    }

    if (!query.exec()) return false;
    select();
    return true;
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

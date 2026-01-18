#include "orderitemmodel.h"
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QDebug>

OrderItemModel::OrderItemModel(QObject* parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
    setTable("order_item");
    setEditStrategy(OnManualSubmit);

    m_idCol      = fieldIndex("id");
    m_orderIdCol = fieldIndex("order_id");
    m_productIdCol = fieldIndex("product_id");
    m_qtyCol     = fieldIndex("quantity");
    m_priceCol   = fieldIndex("unit_price");
    m_stateCol   = fieldIndex("service_state");

    // Start empty until an Order ID is set
    setFilter("1=0");
}

auto OrderItemModel::setOrderId(const QString& orderId) -> void
{
    if (m_currentOrderId == orderId) return;

    m_currentOrderId = orderId;
    setFilter(QString("order_id = '%1'").arg(m_currentOrderId));
    if (!select()) {
        qCritical() << "OrderItem select failed:" << lastError().text();
    }
}

auto OrderItemModel::roleNames() const -> QHash<int, QByteArray>
{
    return {
        { IdRole, "id" },
        { OrderIdRole, "orderId" },
        { ProductIdRole, "productId" },
        { QuantityRole, "quantity" },
        { UnitPriceRole, "unitPrice" },
        { ServiceStateRole, "serviceState" },
        { RowTotalRole, "rowTotal" }
    };
}

[[nodiscard]] auto OrderItemModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid()) return {};
    if (role < Qt::UserRole) return QSqlTableModel::data(index, role);

    const int row = index.row();

    // Computed role
    if (role == RowTotalRole) {
        double qty = record(row).value(m_qtyCol).toDouble();
        double price = record(row).value(m_priceCol).toDouble();
        return qty * price;
    }

    switch (role) {
    case IdRole:           return QSqlTableModel::data(this->index(row, m_idCol));
    case OrderIdRole:      return QSqlTableModel::data(this->index(row, m_orderIdCol));
    case ProductIdRole:    return QSqlTableModel::data(this->index(row, m_productIdCol));
    case QuantityRole:     return QSqlTableModel::data(this->index(row, m_qtyCol));
    case UnitPriceRole:    return QSqlTableModel::data(this->index(row, m_priceCol));
    case ServiceStateRole: return QSqlTableModel::data(this->index(row, m_stateCol));
    default: return {};
    }
}

auto OrderItemModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    if (!index.isValid()) return false;
    QSqlRecord rec = record(index.row());

    switch (role) {
    case QuantityRole:     rec.setValue(m_qtyCol, value); break;
    case UnitPriceRole:    rec.setValue(m_priceCol, value); break;
    case ServiceStateRole: rec.setValue(m_stateCol, value); break;
    default: return false;
    }

    if (setRecord(index.row(), rec)) {
        emit dataChanged(index, index, {role, RowTotalRole}); // Total changes if qty/price changes
        return submitAll();
    }
    return false;
}

auto OrderItemModel::addItem(const QString& orderId, const QString& productId, double quantity, double unitPrice) -> bool
{
    QSqlQuery query(database());
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare(R"(
        INSERT INTO order_item (id, order_id, product_id, quantity, unit_price, service_state)
        VALUES (:id, :oid, :pid, :qty, :price, 'ordered')
    )");

    query.bindValue(":id", newId);
    query.bindValue(":oid", orderId);
    query.bindValue(":pid", productId);
    query.bindValue(":qty", quantity);
    query.bindValue(":price", unitPrice);

    if (!query.exec()) {
        qCritical() << "Failed to add item:" << query.lastError().text();
        return false;
    }

    // Only refresh if we are currently looking at this order
    if (m_currentOrderId == orderId) {
        select();
    }
    return true;
}

auto OrderItemModel::updateQuantity(const QString& itemId, double quantity) -> bool
{
    QSqlQuery query(database());
    query.prepare("UPDATE order_item SET quantity = :qty WHERE id = :id");
    query.bindValue(":qty", quantity);
    query.bindValue(":id", itemId);

    if (!query.exec()) return false;
    return select();
}

auto OrderItemModel::updateServiceState(const OrderId& itemId, const ServiceState& state) -> bool
{
    QSqlQuery query(database());
    query.prepare("UPDATE order_item SET service_state = :state WHERE id = :id");
    query.bindValue(":state", state.toString());
    query.bindValue(":id", itemId.value);

    if (!query.exec()) return false;
    return select();
}

auto OrderItemModel::removeItem(const QString& itemId) -> bool
{
    QSqlQuery query(database());
    query.prepare("DELETE FROM order_item WHERE id = :id");
    query.bindValue(":id", itemId);

    if (!query.exec()) return false;
    return select();
}

auto OrderItemModel::calculateOrderTotal() const -> double
{
    double total = 0.0;
    for (int i = 0; i < rowCount(); ++i) {
        double qty = record(i).value(m_qtyCol).toDouble();
        double price = record(i).value(m_priceCol).toDouble();
        total += (qty * price);
    }
    return total;
}

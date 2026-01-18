#include "orderitemmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>

OrderItemModel::OrderItemModel(QObject *parent) : QAbstractListModel(parent) {}

auto OrderItemModel::rowCount(const QModelIndex &parent) const -> int {
    return parent.isValid() ? 0 : static_cast<int>(m_stagedItems.size());
}

auto OrderItemModel::roleNames() const -> QHash<int, QByteArray> {
    return {
        {NameRole, "name"},
        {QuantityRole, "quantity"},
        {UnitPriceRole, "unitPrice"},
        {TotalPriceRole, "totalPrice"},
        {ModifiersRole, "modifiers"},
        {ProductIdRole, "productId"}
    };
}

auto OrderItemModel::data(const QModelIndex &index, int role) const -> QVariant {
    if (!index.isValid() || index.row() >= m_stagedItems.size()) return {};

    const auto &item = m_stagedItems.at(index.row());
    switch (role) {
    case NameRole:      return item.product.internalProductName;
    case QuantityRole:  return item.quantity;
    case UnitPriceRole: return item.finalUnitPrice;
    case TotalPriceRole: return item.finalUnitPrice * item.quantity;
    case ModifiersRole: return item.modifiersJson;
    case ProductIdRole: return item.product.id;
    default: return {};
    }
}

void OrderItemModel::addItem(const Product &p) {
    auto it = std::find_if(m_stagedItems.begin(), m_stagedItems.end(),
                           [&p](const StagedItem &item) {
                               return item.product.id == p.id;
                           });

    if (it != m_stagedItems.end()) {
        it->quantity += 1.0;
        int row = static_cast<int>(std::distance(m_stagedItems.begin(), it));
        auto modelIndex = index(row, 0);
        emit dataChanged(modelIndex, modelIndex, {QuantityRole, TotalPriceRole});
    } else {
        int row = static_cast<int>(m_stagedItems.size());
        beginInsertRows(QModelIndex(), row, row);
        m_stagedItems.append({p, 1.0, p.defaultSellingPrice, ""});
        endInsertRows();
        emit countChanged();
    }
    recalculateTotal();
}

auto OrderItemModel::removeItem(int index) -> bool {
    if (index < 0 || index >= m_stagedItems.size()) return false;
    beginRemoveRows(QModelIndex(), index, index);
    m_stagedItems.removeAt(index);
    endRemoveRows();

    recalculateTotal();
    emit countChanged();
    return true;
}

auto OrderItemModel::updateQuantity(int index, double qty) -> bool {
    if (index < 0 || index >= m_stagedItems.size()) return true;
    m_stagedItems[index].quantity = qty;
    auto idx = this->index(index, 0);
    emit dataChanged(idx, idx, {QuantityRole, TotalPriceRole});
    recalculateTotal();
    return true;
}

void OrderItemModel::clear() {
    beginResetModel();
    m_stagedItems.clear();
    endResetModel();

    m_cachedTotal = 0.0;
    emit countChanged();
    emit totalChanged();
}

auto OrderItemModel::totalAmount() const -> double {
    return m_cachedTotal;
}
void OrderItemModel::recalculateTotal() {
    double tempTotal = 0.0;
    for (const auto &item : std::as_const(m_stagedItems)) {
        tempTotal += (item.finalUnitPrice * item.quantity);
    }

    if (!qFuzzyCompare(m_cachedTotal, tempTotal)) {
        m_cachedTotal = tempTotal;
        // ADD THIS:
        qDebug() << "OrderItemModel :: New Total Calculated:" << m_cachedTotal
                 << " | Item Count:" << m_stagedItems.size();

        emit totalChanged();
    }
}
auto OrderItemModel::addOrderItem(const QString &orderId) -> bool {
    if (m_stagedItems.isEmpty()) return true;

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO order_item (
            id, order_id, product_id, quantity, unit_price, modifiers
        ) VALUES (
            :id, :oid, :pid, :qty, :price, :mods
        )
    )");

    for (const auto &item : std::as_const(m_stagedItems)) {
        query.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
        query.bindValue(":oid", orderId);
        query.bindValue(":pid", item.product.id);
        query.bindValue(":qty", item.quantity);
        query.bindValue(":price", item.finalUnitPrice);
        query.bindValue(":mods", item.modifiersJson);

        if (!query.exec()) {
            qCritical() << "Failed to insert order item:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

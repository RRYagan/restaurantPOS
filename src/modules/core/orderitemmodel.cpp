#include "orderitemmodel.h"
#include "../../ui/controllers/salesviewcontroller.h"

#include <QSqlQuery>
#include <QSqlError>

OrderItemModel::OrderItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

auto OrderItemModel::setSourceData(const QList<StagedItem>* source) -> void {
    beginResetModel();
    m_stagedItems = source;
    endResetModel();
}

auto OrderItemModel::rowCount(const QModelIndex &parent) const -> int {
    if (parent.isValid() || !m_stagedItems) return 0;
    return static_cast<int>(m_stagedItems->size());
}

auto OrderItemModel::roleNames() const -> QHash<int, QByteArray> {
    QHash<int, QByteArray> roles;
    roles[NameRole]      = "name";
    roles[QuantityRole]  = "quantity";
    roles[UnitPriceRole] = "unitPrice";
    roles[TotalPriceRole] = "totalPrice";
    roles[ModifiersRole] = "modifiers";
    roles[ProductIdRole] = "productId";
    return roles;
}
auto OrderItemModel::data(const QModelIndex &index, int role) const -> QVariant {
    if (!m_stagedItems || !index.isValid()){
        return {};
    }

    const int row = index.row();
    if (row < 0 || row >= m_stagedItems->size())
    {
        return {};
    }

    const auto &item = m_stagedItems->at(row);


    switch (role) {
    case NameRole:
        return item.product.internalProductName; // Check if this is empty
    case QuantityRole:
        return item.quantity;
    case UnitPriceRole:
        return item.finalUnitPrice;
    case TotalPriceRole:
        return item.finalUnitPrice * item.quantity;
    case ModifiersRole:
        return item.modifiersJson;
    case ProductIdRole:
        return item.product.id;
    default:
        return {};
    }
}

auto OrderItemModel::addOrderItem(const QVariantMap& data) -> bool {
    QSqlQuery query;
    // Using Raw String Literal for clarity
    query.prepare(R"(
        INSERT INTO order_item (
            id,
            order_id,
            product_id,
            quantity,
            unit_price,
            modifiers
        ) VALUES (
            :id, :oid, :pid, :qty, :price, :mods
        )
    )");

    query.bindValue(":id", data.value("id").toString());
    query.bindValue(":oid", data.value("order_id").toString());
    query.bindValue(":pid", data.value("product_id").toString());
    query.bindValue(":qty", data.value("quantity").toDouble());
    query.bindValue(":price", data.value("unit_price").toDouble());
    query.bindValue(":mods", data.value("modifiers").toString());

    if (!query.exec()) {
        qCritical() << "Item DB Insert Error:" << query.lastError().text();
        return false;
    }

    return true;
}

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
        {TaxAmountRole, "taxAmount"},
        {ModifiersRole, "modifiers"},
        {ProductIdRole, "productId"}
    };
}

auto OrderItemModel::data(const QModelIndex &index, int role) const -> QVariant {
    if (!index.isValid() || index.row() >= m_stagedItems.size()) return {};

    const auto &item = m_stagedItems.at(index.row());
    Money lineTotal = item.finalUnitPrice * item.quantity;
    Money lineTaxAmount = item.taxAmountPerUnit * item.quantity;

    switch (role) {
    case NameRole:      {
        return item.product.internalProductName;
    }
    case QuantityRole:  {
        return item.quantity;
    }
    case UnitPriceRole: {
        return QVariant(static_cast<double>(item.finalUnitPrice.toKSH()));
    }
    case TotalPriceRole: {
        return QVariant::fromValue(lineTotal.toKSH());
    }
    case TaxAmountRole: {
        return QVariant(static_cast<double>(lineTaxAmount.toKSH()));
    }
    case ModifiersRole: {
        return item.modifiersJson;
    }
    case ProductIdRole: {
        return item.product.id;
    }
    default: {
        return {};
    }
    }
}

void OrderItemModel::addItem(const Product &p) {
    // Check if item exists to increment quantity
    auto it = std::find_if(m_stagedItems.begin(), m_stagedItems.end(),
                           [&](const StagedItem &item) { return item.product.id == p.id; });

    if (it != m_stagedItems.end()) {
        it->quantity += 1.0;
        int row = static_cast<int>(std::distance(m_stagedItems.begin(), it));
        auto idx = index(row, 0);
        emit dataChanged(idx, idx, {QuantityRole, TotalPriceRole, TaxAmountRole});


    } else {
        beginInsertRows(QModelIndex(), m_stagedItems.size(), m_stagedItems.size());
        m_stagedItems.append({
            p,                      // Trusted Product data
            1.0,                    // Quantity
            p.defaultSellingPrice,  // Unit Price (Money)
            "",                     // Modifiers
            p.taxClassificationCode,
            p.taxAmount             // Tax (Money)
        });
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
    emit dataChanged(idx, idx, {QuantityRole, TotalPriceRole, TaxAmountRole});
    recalculateTotal();
    return true;
}

void OrderItemModel::clear() {
    beginResetModel();
    m_stagedItems.clear();
    endResetModel();

    m_cachedTotal = Money(0);
    emit countChanged();
    emit totalChanged();
}

auto OrderItemModel::totalTaxAmount() const -> Money {
    return m_cachedTaxTotal;
}
auto OrderItemModel::totalAmount() const -> Money {
    return m_cachedTotal;
}

void OrderItemModel::recalculateTotal() {
    Money tempTotal(0);
    Money tempTax(0);

    for (const auto &item : std::as_const(m_stagedItems)) {
        Money lineTotal = item.finalUnitPrice * item.quantity;
        tempTotal = tempTotal + lineTotal;
        // Correctly accumulate tax for all items in the list
        Money lineTaxTotal = item.taxAmountPerUnit * item.quantity;
        tempTax = tempTax + lineTaxTotal;
    }
    bool totalChangedFlag = (m_cachedTotal != tempTotal);
    bool taxChangedFlag = (m_cachedTaxTotal != tempTax);

    if (totalChangedFlag || taxChangedFlag) {
        m_cachedTotal = tempTotal;
        m_cachedTaxTotal = tempTax;

        // qDebug() << "Totals Updated - Amount:" << m_cachedTotal << "Tax:" << m_cachedTaxTotal;

        // This signal triggers the SalesViewController to notify QML
        emit totalChanged();
    }
}

auto OrderItemModel::submitOrderItem(const QString &orderId) -> bool {
    if (m_stagedItems.isEmpty()) return true;

    QSqlQuery query;
    query.prepare(R"(
    INSERT INTO order_item (
        id, order_id, product_id, quantity, unit_price,
        kra_item_code, tax_classification_code, tax_amount
    ) VALUES (
        :id, :oid, :pid, :qty, :price, :kcode, :tax_code, :tax_amt
    )
)");

    for (const auto &item : std::as_const(m_stagedItems)) {
        query.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
        query.bindValue(":oid", orderId);
        query.bindValue(":pid", item.product.id);
        query.bindValue(":qty", item.quantity);
        query.bindValue(":price", static_cast<qlonglong>(item.finalUnitPrice.cents));
        query.bindValue(":kcode", item.product.kraItemCode);
        query.bindValue(":tax_code", item.product.taxClassificationCode);

        Money lineTaxTotal = item.taxAmountPerUnit * item.quantity;
        query.bindValue(":tax_amt", static_cast<qlonglong>(lineTaxTotal.cents));
        // query.bindValue(":mods", item.modifiersJson);

        if (!query.exec()) {
            qCritical() << "Failed to insert order item:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

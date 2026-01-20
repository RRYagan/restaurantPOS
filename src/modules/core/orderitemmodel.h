#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>
// #include "product.h"
#include "productmodel.h"
#include "types.h"


class OrderItemModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum OrderItemRoles: std::uint16_t {
        NameRole = Qt::UserRole + 1,
        QuantityRole,
        UnitPriceRole,
        TotalPriceRole,
        TaxAmountRole,
        ModifiersRole,
        ProductIdRole
    };

    explicit OrderItemModel(QObject *parent = nullptr);

    /* Model Overrides */
    [[nodiscard]] auto rowCount(const QModelIndex &parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex &index, int role) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    [[nodiscard]] auto count() const -> int {
        return static_cast<int>(m_stagedItems.size());
    }

    [[nodiscard]] auto isEmpty() const -> bool {
        return m_stagedItems.isEmpty();
    }

    void addItem(const Product &p);
    auto removeItem(int index) -> bool;
    auto updateQuantity(int index, double qty) -> bool;
    void clear();

    /* Database Persistence */
    auto submitOrderItem(const QString &orderId) -> bool;

    /* Getters for calculation */
    [[nodiscard]] auto stagedItems() const -> const QList<StagedItem>& { return m_stagedItems; }
    [[nodiscard]] auto totalAmount() const -> Money;
    [[nodiscard]] auto totalTaxAmount() const -> Money;

signals:
    void countChanged();
    void totalChanged();

private:
    QList<StagedItem> m_stagedItems;
    Money m_cachedTotal;
    Money m_cachedTaxTotal;

    /* Helper to update the cache safely */
    void recalculateTotal();
};

#endif

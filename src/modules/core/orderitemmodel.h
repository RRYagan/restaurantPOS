#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include <QAbstractListModel>
#include <QList>

// Forward declaration to avoid circular dependency
struct StagedItem;

class OrderItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum OrderItemRoles {
        NameRole = Qt::UserRole + 1,
        QuantityRole,
        UnitPriceRole,
        TotalPriceRole,
        ModifiersRole,
        ProductIdRole
    };

    explicit OrderItemModel(QObject *parent = nullptr);

    // Public wrappers for protected methods
    auto prepareForAddition(int row) -> void { beginInsertRows(QModelIndex(), row, row); }
    auto finishAddition() -> void { endInsertRows(); }

    auto prepareForRemoval(int row) -> void { beginRemoveRows(QModelIndex(), row, row); }
    auto finishRemoval() -> void { endRemoveRows(); }

    auto notifyRowChanged(int row, const QVector<int>& roles = {}) -> void {
        auto idx = index(row, 0);
        emit dataChanged(idx, idx, roles);
    }
    // Links the model to the live list in SalesViewController
    auto setSourceData(const QList<StagedItem>* source) -> void;

    // Required overrides
    [[nodiscard]] auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    [[nodiscard]] auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant override;
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

    auto addOrderItem(const QVariantMap& data) -> bool;

private:
    const QList<StagedItem>* m_stagedItems = nullptr;
};

#endif

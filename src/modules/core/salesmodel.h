#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "types.h"

class SalesModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString tableNumber READ tableNumber WRITE setTableNumber NOTIFY headerChanged)
    Q_PROPERTY(QString currentOrderId READ currentOrderId NOTIFY headerChanged)
    Q_PROPERTY(double totalAmount READ totalAmount NOTIFY totalsChanged)
    Q_PROPERTY(double totalTaxAmount READ totalTaxAmount NOTIFY totalsChanged)

public:
    enum OrderItemRoles : std::uint16_t {
        NameRole = Qt::UserRole + 1,
        QuantityRole,
        UnitPriceRole,
        TotalPriceRole,
        TaxAmountRole,
        ProductIdRole
    };

    explicit SalesModel(QObject *parent = nullptr);

    // Core Logic (Using cohesive types)
    void addItem(const StagedItem &item);
    void removeItem(int index);
    void updateQuantity(int index, double qty);
    void clear();

    // Persistence & State
    auto loadOrder(const QString &orderId) -> bool;
    [[nodiscard]] auto submitOrder() -> QString;

    // Accessors (Expressing Intent)
    [[nodiscard]] auto tableNumber() const -> QString { return m_tableNumber; }
    void setTableNumber(const QString &t);

    [[nodiscard]] auto currentOrderId() const -> QString { return m_currentOrderId; }
    [[nodiscard]] auto totalAmount() const -> double { return m_cachedTotal.toKSH(); }
    [[nodiscard]] auto totalTaxAmount() const -> double { return m_cachedTaxTotal.toKSH(); }
    [[nodiscard]] auto count() const -> int { return static_cast<int>(m_items.size()); }

    // QAbstractListModel overrides
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const  override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Data Queries
    [[nodiscard]] QList<Order> fetchAllOrders() const;
    [[nodiscard]] QList<KitchenTicket> fetchKitchenQueue() const;
    bool updateItemStatus(const QString &orderId, const QString &status);

signals:
    void totalsChanged();
    void headerChanged();
    void countChanged();
    void orderStatusChanged();
    void inventorydbModified();

private:
    void recalculateTotals();
    [[nodiscard]] bool reduceInventory(QSqlDatabase &db);

    QList<StagedItem> m_items;
    QString m_tableNumber = "1";
    QString m_waiterId = "admin";
    QString m_currentOrderId;
    Money m_cachedTotal{0};
    Money m_cachedTaxTotal{0};
};

#endif // SALESMODEL_H

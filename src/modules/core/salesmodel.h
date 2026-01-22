#ifndef SALESMODEL_H
#define SALESMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "types.h"

struct KitchenTicket {
    QString orderId;
    QString tableNumber;
    QString timestamp;
    QString itemsSummary;
    QString itemIds;
};

class SalesModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString tableNumber READ tableNumber WRITE setTableNumber NOTIFY headerChanged)
    Q_PROPERTY(QString currentOrderId READ currentOrderId NOTIFY headerChanged)
    Q_PROPERTY(double totalAmount READ totalAmount NOTIFY totalsChanged)
    Q_PROPERTY(double totalTaxAmount READ totalTaxAmount NOTIFY totalsChanged)

public:
    enum OrderItemRoles: std::uint16_t {
        NameRole = Qt::UserRole + 1,
        QuantityRole,
        UnitPriceRole,
        TotalPriceRole,
        TaxAmountRole,
        // ModifiersRole,
        ProductIdRole
    };

    explicit SalesModel(QObject *parent = nullptr);

    // --- High-Level Logic (Deep Interface) ---
    void addItem(const Product &p);
    void removeItem(int index);
    void updateQuantity(int index, double qty);
    void clear();

    // Persistence
    auto loadOrder(const QString &orderId) -> bool;
    auto submitOrder() -> QString; // Returns ID on success, empty on fail

    // Accessors
    [[nodiscard]] auto tableNumber() const -> QString { return m_tableNumber; }
    void setTableNumber(const QString &t) { if(m_tableNumber != t) { m_tableNumber = t; emit headerChanged(); } }
    [[nodiscard]] auto currentOrderId() const -> QString { return m_currentOrderId; }
    [[nodiscard]] auto totalAmount() const -> double { return m_cachedTotal.toKSH(); }
    [[nodiscard]] auto totalTaxAmount() const -> double { return m_cachedTaxTotal.toKSH(); }
    [[nodiscard]] int count() const { return static_cast<int>(m_items.size()); }

    // Model Implementation
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    QList<Order> fetchAllOrders();
    QList<KitchenTicket> fetchKitchenQueue();
    bool updateItemStatus(const QString &itemId, const QString &status);

signals:
    void totalsChanged();
    void headerChanged();
    void countChanged();
    void orderStatusChanged();

    void inventorydbModified(); /*inventory changed broadcast */

private:
    void recalculateTotals();
    bool reduceInventory(QSqlDatabase &db);

    QList<StagedItem> m_items;
    QString m_tableNumber = "1";
    QString m_waiterId = "admin";
    QString m_currentOrderId;
    Money m_cachedTotal;
    Money m_cachedTaxTotal;
};

#endif

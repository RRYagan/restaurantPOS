#ifndef ORDERTABLEMODEL_H
#define ORDERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>
#include <QVector>
#include <QDateTime>

struct Order {
    QString id;
    int tableNumber;
    QString waiterId;
    QString status;
    int totTaxableAmtCents;
    int totTaxAmtCents;
    int totAmtCents;
    QString sdcReceiptNo;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct OrderItem {
    QString id;
    QString orderId;
    QString menuItemId;
    QString name;
    double quantity;
    int unitPriceCents;
    int taxableAmtCents;
    int taxAmtCents;
    QString taxType;
    // Helper constructor for easy creation
    OrderItem(QString id = "", QString nm = "", double qty = 1.0, int price = 0, QString tax = "B")
        : menuItemId(id), name(nm), quantity(qty), unitPriceCents(price), taxType(tax) {}
};

class OrderTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum OrderRoles {
        IdRole = Qt::UserRole + 1,
        TableNumberRole,
        WaiterIdRole,
        StatusRole,
        TaxableAmountRole,
        TaxAmountRole,
        TotalAmountRole,
        ReceiptNoRole,
        CreatedAtRole,
        UpdatedAtRole
    };

    enum OrderItemRoles {
        ItemNameRole = Qt::UserRole + 20,
        ItemQtyRole,
        ItemPriceRole,
        ItemSubtotalRole
    };

    explicit OrderTableModel(QObject* parent = nullptr);


    // Header & Data
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Orders API
    bool loadAll();
    // Q_INVOKABLE bool loadByStatus(const QString& status);


    // Orders item API
    void addItem(const OrderItem& newItem);
    void removeItem(int index);
    void clearCurrentOrder();
    bool submitOrder();
    // void clearCurrentOrder();
    void updateQuantity(int index, double newQuantity); // Standard C++ public method

    int currentTotalCents() const { return m_currentTotalCents; }


signals:
    void totalsChanged();
private:
    QSqlDatabase m_db;
    QVector<Order> m_orders;
    QVector<OrderItem> m_currentItems; // Items in the "Cart"
    int m_currentTotalCents = 0;

    // function
    void calculateTotals();
};

#endif // ORDERTABLEMODEL_H

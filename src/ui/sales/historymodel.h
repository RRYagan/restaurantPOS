#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractTableModel>
#include <QSqlQuery>
#include <orderitem.h>
#include <QtQml/qqmlregistration.h>

class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum HistoryRoles {
        OrderIdRole = Qt::UserRole + 1,
        TableRole,
        DateRole,
        DisplayTitleRole // For the "Order #1 (Table 1)" string
    };

    explicit HistoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadOrderHistory();
    Q_INVOKABLE void viewOrderDetails(int orderId);
    QString totalFormatted() const;

signals:
    void totalChanged();
private:
    struct HistoryRecord {
        int id;
        int tableNumber;
        QString createdAt;
    };
    QList<HistoryRecord> m_history;
    QList<OrderItem> m_items;
    void calculateTotal();
    Money m_totalMoney;
};

#endif

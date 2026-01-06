#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractTableModel>
#include <QSqlQuery>
#include <orderitem.h>
#include <QtQml/qqmlregistration.h>

class OrdersView : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum HistoryRoles {
        OrderIdRole = Qt::UserRole,
        TableRole,
        DateRole,
        DisplayTitleRole,

    };

    explicit OrdersView(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadOrderHistory();

private:
    struct HistoryRecord {
        QString id;
        int tableNumber;
        QString createdAt;
    };
    QList<HistoryRecord> m_history;
};

#endif

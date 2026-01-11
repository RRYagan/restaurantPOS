// #ifndef ORDERSYVIEW_H
// #define ORDERSVIEW_H

// #include <QAbstractTableModel>
// #include <QSqlQuery>
// // #include <orderitem.h>
// #include <universalfilterproxy.h>
// #include <QtQml/qqmlregistration.h>

// class OrdersView : public QAbstractTableModel
// {
//     Q_OBJECT
//     QML_ELEMENT

//     Q_PROPERTY(UniversalFilterProxy* proxy READ proxy CONSTANT)

// public:
//     enum HistoryRoles {
//         OrderIdRole = Qt::UserRole,
//         TableRole,
//         DateRole,
//         DisplayTitleRole,

//     };

//     explicit OrdersView(QObject *parent = nullptr);
//     UniversalFilterProxy* proxy() const { return m_proxy; }


//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//     int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//     QHash<int, QByteArray> roleNames() const override;

//     Q_INVOKABLE void loadOrders();

// private:
//     struct OrdersRecord {
//         QString id;
//         int tableNumber;
//         QString createdAt;
//     };
//     QList<OrdersRecord> m_history;
//     UniversalFilterProxy* m_proxy; // Add this
// };

// #endif

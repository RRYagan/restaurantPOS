// #include "salesviewcontroller.h"
// #include <QSqlDatabase>
// #include <QSqlQuery>
// #include <QSqlError>
// #include <QUuid>
// #include <QDateTime>
// #include <QDebug>

// SalesViewController::SalesViewController(QObject* parent)
//     : QObject(parent)
//     , m_orderModel(new OrderModel(this))
//     , m_itemModel(new OrderItemModel(this))
//     ,m_productModel(new ProductModel(this))
// {
//     // When the model count changes, tell QML the itemCount property changed
//     connect(m_itemModel, &OrderItemModel::countChanged,
//             this, &SalesViewController::itemCountChanged);
//     // Connect model signals to controller signals for QML property updates
//     connect(m_itemModel, &OrderItemModel::totalChanged, this, &SalesViewController::orderChanged);
// }

// auto SalesViewController::addItem(const QString& productId) -> bool {
//     // 1. Fetch fresh, trusted data from the ProductModel/DB
//     Product p = m_productModel->getProductById(productId);

//     if (p.id.isEmpty()) return false;
//     m_itemModel->addItem(p);
//     return true;
// }

// auto SalesViewController::removeItem(int index) -> bool {
//     m_itemModel->removeItem(index);
//     return true;
// }

// auto SalesViewController::updateQuantity(int index, double qty) -> bool {
//     m_itemModel->updateQuantity(index, qty);
//     return true;
// }

// auto SalesViewController::clearOrder() -> void {
//     m_itemModel->clear();
//     m_currentOrderId.clear();
//     emit orderIdChanged();
// }

// auto SalesViewController::makeOrder() -> bool {
//     if (m_itemModel->rowCount(QModelIndex()) == 0) return false;
//     m_isBusy = true;
//     emit isBusyChanged();
//     m_currentOrderId = QUuid::createUuid().toString(QUuid::WithoutBraces);
//     // 2. Add all items from the OrderItemModel
//     if (!m_itemModel->submitOrderItem(m_currentOrderId)) {
//         // db.rollback();
//         return false;
//     }
//     /* emit order added successfully */

//     // 1. Create the Parent Order

//     QVariantMap orderData;
//     orderData.insert("id", m_currentOrderId);
//     orderData.insert("order_status", "open");
//     orderData.insert("table_number", "1");
//     orderData.insert("waiter_id", "admi");

//     if (!m_orderModel->addOrder(orderData)) {
//         // db.rollback();
//         return false;
//     }

//     emit orderIdChanged();
//     m_isBusy = false;
//     emit isBusyChanged();

//     return true;
// }

// // auto SalesViewController::finalizeSale(const QVariantMap& paymentData) -> bool {
// //     if (m_currentOrderId.isEmpty()) return false;

// //     QSqlDatabase db = QSqlDatabase::database();
// //     db.transaction();

// //     Money total = m_itemModel->totalAmount();

// //     qlonglong totalCents = total.cents;
// //     qlonglong net = total.cents;
// //     QSqlQuery sq;
// //     sq.bindValue(":total", totalCents);
// //     sq.bindValue(":net", net);

// //     sq.prepare(R"(
// //         INSERT INTO sale (
// //             id, order_id, sale_transaction_date, net_amount,
// //             tax_amount, total_amount, payment_method_id
// //         ) VALUES (
// //             :id, :oid, :date, :net, :tax, :total, :pmid
// //         )
// //     )");

// //     sq.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
// //     sq.bindValue(":oid", m_currentOrderId);
// //     sq.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
// //     sq.bindValue(":net", net);
// //     sq.bindValue(":tax", total.cents);
// //     sq.bindValue(":total", total.cents);
// //     sq.bindValue(":pmid", paymentData.value("payment_method_id").toInt());

// //     if (!sq.exec()) {
// //         qCritical() << "Finalize Sale Error:" << sq.lastError().text();
// //         db.rollback();
// //         return false;
// //     }

// //     // Update Order Status to Closed
// //     QSqlQuery uq;
// //     uq.prepare(R"(UPDATE customer_order SET order_status = 'closed' WHERE id = :id)");
// //     uq.bindValue(":id", m_currentOrderId);

// //     if (uq.exec() && db.commit()) {
// //         clearOrder();
// //         return true;
// //     }

// //     db.rollback();
// //     return false;
// // }

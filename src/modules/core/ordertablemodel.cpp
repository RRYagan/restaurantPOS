#include "ordertablemodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

OrderTableModel::OrderTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_db = DatabaseManager::instance().database();
    loadAll();
}

int OrderTableModel::rowCount(const QModelIndex&) const { return m_orders.size(); }
int OrderTableModel::columnCount(const QModelIndex&) const { return 7; } // Adjust based on columns needed in TableView

QHash<int, QByteArray> OrderTableModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "orderId";
    roles[TableNumberRole] = "tableNumber";
    roles[WaiterIdRole] = "waiterId";
    roles[StatusRole] = "status";
    roles[TaxableAmountRole] = "taxableAmount";
    roles[TaxAmountRole] = "taxAmount";
    roles[TotalAmountRole] = "totalAmount";
    roles[ReceiptNoRole] = "receiptNo";
    roles[CreatedAtRole] = "createdAt";
    roles[UpdatedAtRole] = "updatedAt";
    return roles;
}

QVariant OrderTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_orders.size()) return QVariant();

    const Order& order = m_orders[index.row()];

    // Handle QML Roles
    switch (role) {
    case IdRole:            return order.id;
    case TableNumberRole:   return order.tableNumber;
    case WaiterIdRole:      return order.waiterId;
    case StatusRole:        return order.status;
    case TotalAmountRole:   return order.totAmtCents;
    case ReceiptNoRole:     return order.sdcReceiptNo;
    case CreatedAtRole:     return order.createdAt;
    }

    // Handle TableView Columns (Qt::DisplayRole)
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return order.id;
        case 1: return order.tableNumber;
        case 2: return order.status;
        case 3: return order.totAmtCents;
        case 4: return order.sdcReceiptNo;
        case 5: return order.createdAt.toString("yyyy-MM-dd HH:mm");
        case 6: return order.waiterId;
        }
    }
    return QVariant();
}

QVariant OrderTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();
    switch (section) {
    case 0: return "ID";
    case 1: return "Table";
    case 2: return "Status";
    case 3: return "Total (Cents)";
    case 4: return "Receipt";
    case 5: return "Date";
    case 6: return "Waiter";
    }
    return QVariant();
}

bool OrderTableModel::loadAll() {
    beginResetModel();
    m_orders.clear();
    QSqlQuery q("SELECT id, table_number, waiter_id, status, tot_taxbl_amt_cents, "
                "tot_tax_amt_cents, tot_amt_cents, sdc_receipt_no, created_at, updated_at "
                "FROM orders ORDER BY created_at DESC", m_db);

    while (q.next()) {
        Order o;
        o.id = q.value(0).toString();
        o.tableNumber = q.value(1).toInt();
        o.waiterId = q.value(2).toString();
        o.status = q.value(3).toString();
        o.totTaxableAmtCents = q.value(4).toInt();
        o.totTaxAmtCents = q.value(5).toInt();
        o.totAmtCents = q.value(6).toInt();
        o.sdcReceiptNo = q.value(7).toString();
        o.createdAt = q.value(8).toDateTime();
        o.updatedAt = q.value(9).toDateTime();
        m_orders.append(o);
    }
    endResetModel();
    return true;
}

void OrderTableModel::addItem(const OrderItem& newItem)
{
    // 1. Check if item already exists in current order
    for (int i = 0; i < m_currentItems.size(); ++i) {
        if (m_currentItems[i].menuItemId == newItem.menuItemId) {
            // Increment quantity based on the passed item's qty (usually 1.0)
            m_currentItems[i].quantity += newItem.quantity;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx);
            calculateTotals();
            return;
        }
    }

    // 2. Otherwise add new item
    beginInsertRows(QModelIndex(), m_currentItems.size(), m_currentItems.size());
    m_currentItems.append(newItem);
    endInsertRows();

    calculateTotals();
}

void OrderTableModel::removeItem(int index)
{
    if (index < 0 || index >= m_currentItems.size()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_currentItems.removeAt(index);
    endRemoveRows();

    calculateTotals();
}

void OrderTableModel::calculateTotals()
{
    int total = 0;
    for (const auto& item : m_currentItems) {
        total += (item.unitPriceCents * item.quantity);
    }

    if (m_currentTotalCents != total) {
        m_currentTotalCents = total;
        // This triggers the 'orderChanged' signal in the ViewController
        // which in turn updates 'totalFormatted' in the UI
        emit totalsChanged();
    }
}

bool OrderTableModel::submitOrder() {
    m_db.transaction();

    QSqlQuery q(m_db);
    QString orderId = QUuid::createUuid().toString();

    // 1. Insert into orders table
    q.prepare("INSERT INTO orders (id, tot_amt_cents, status) VALUES (?, ?, 'PAID')");
    q.addBindValue(orderId);
    q.addBindValue(m_currentTotalCents);
    if(!q.exec()) { m_db.rollback(); return false; }

    // 2. Insert all items into order_items table
    q.prepare("INSERT INTO order_items (id, order_id, menu_item_id, name, quantity, unit_price_cents) "
              "VALUES (?, ?, ?, ?, ?, ?)");
    for(const auto& item : m_currentItems) {
        q.addBindValue(QUuid::createUuid().toString());
        q.addBindValue(orderId);
        q.addBindValue(item.menuItemId);
        q.addBindValue(item.name);
        q.addBindValue(item.quantity);
        q.addBindValue(item.unitPriceCents);
        if(!q.exec()) { m_db.rollback(); return false; }
    }

    return m_db.commit();
}

void OrderTableModel::clearCurrentOrder()
{
    if (m_currentItems.isEmpty()) return;

    beginResetModel();
    m_currentItems.clear();
    m_currentTotalCents = 0;
    endResetModel();

    emit totalsChanged();
}

void OrderTableModel::updateQuantity(int index, double newQuantity)
{
    // 1. Validate the index range
    if (index < 0 || index >= m_currentItems.size()) {
        return;
    }

    // 2. Prevent invalid quantities (optional: handled by controller, but safe here)
    if (newQuantity <= 0) {
        removeItem(index);
        return;
    }

    // 3. Update the quantity in our data vector
    m_currentItems[index].quantity = newQuantity;

    // 4. Notify any attached Views (like your ListView) that this row's data changed
    // We use column 0 and ItemQtyRole to be specific
    QModelIndex idx = createIndex(index, 0);
    emit dataChanged(idx, idx, {ItemQtyRole});

    // 5. Recalculate the grand total and emit totalsChanged()
    calculateTotals();
}

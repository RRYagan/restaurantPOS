#include "salesviewcontroller.h"
#include "salesmodel.h"
#include "productmodel.h"
#include "inventoryviewcontroller.h"

// A simple RAII guard to ensure the 'isBusy' flag is reset on function exit
struct BusyGuard {
    bool& target;
    std::function<void()> onExit;
    ~BusyGuard() { target = false; onExit(); }
};

SalesViewController::SalesViewController(QObject* parent)
    : QObject(parent),
    m_salesModel(std::make_unique<SalesModel>(this)),
    m_orderItemModel(std::make_unique<OrderItemModel>(this)),
    m_productModel(std::make_unique<ProductModel>(this)),
    m_inventoryController(std::make_unique<InventoryViewController>(this)),
    m_genericFilterProxyModel(std::make_unique<GenericFilterProxyModel>(this))
{
    m_genericFilterProxyModel->setSourceModel(m_productModel.get());
    m_genericFilterProxyModel->setCategoryRole(ProductModel::CategoryIdRole);
    m_genericFilterProxyModel->setTypeRole(ProductModel::ProductTypeIdRole);

    // // Configure proxy for Kitchen View needs
    // m_genericFilterProxyModel->setSourceModel(m_orderItemModel.get());

    // // Map the proxy to the roles used for filtering items
    // // Assuming OrderItemModel has StatusRole
    // m_genericFilterProxyModel->setFilterRole(OrderItemModel::StatusRole);


    /* Connect using .get() to access the raw pointer owned by unique_ptr */
    connect(m_salesModel.get(), &SalesModel::totalsChanged, this, &SalesViewController::orderChanged);
    connect(m_salesModel.get(), &SalesModel::countChanged, this, &SalesViewController::itemCountChanged);
    connect(m_salesModel.get(), &SalesModel::inventorydbModified,
            m_inventoryController.get(), &InventoryViewController::refresh);
    connect(m_salesModel.get(), &SalesModel::orderStatusChanged,
            this, &SalesViewController::kitchenDataChanged);
    connect(m_orderItemModel.get(), &OrderItemModel::orderItemStatusChanged,
            this, &SalesViewController::kitchenDataChanged);
}

double SalesViewController::totalAmount() const { return m_salesModel->totalAmount(); }
double SalesViewController::totalTaxFormatted() const { return m_salesModel->totalTaxAmount(); }
int SalesViewController::itemCount() const { return m_salesModel->count(); }

bool SalesViewController::addItem(const QString& productId) {
    const Product p = m_productModel->getProductById(productId);
    if (p.id.isEmpty()) {
        return false;
    }

    // Initialize the StagedItem structure with defaults from the Product
    StagedItem staged;
    staged.product = p;
    staged.quantity = 1.0;
    staged.finalUnitPrice = p.defaultSellingPrice;
    staged.taxClassificationCode = p.taxClassificationCode;
    staged.taxAmountPerUnit = p.taxAmount;
    staged.modifiersJson = "";

    // Now the types match the SalesModel::addItem(const StagedItem&) signature
    m_salesModel->addItem(staged);

    return true;
}

bool SalesViewController::removeItem(int index) {
    m_salesModel->removeItem(index);
    return true;
}

bool SalesViewController::updateQuantity(int index, double qty) {
    m_salesModel->updateQuantity(index, qty);
    return true;
}

void SalesViewController::clearOrder() {
    m_salesModel->clear();
}

bool SalesViewController::makeOrder() {
    m_isBusy = true;
    emit isBusyChanged();

    // RAII: Ensure m_isBusy becomes false when this function exits, no matter what
    BusyGuard guard{ m_isBusy, [this] { emit isBusyChanged(); } };

    const QString id = m_salesModel->submitOrder();
    return !id.isEmpty();
}

bool SalesViewController::editOrder(const QString& orderId) {
    return m_salesModel->loadOrder(orderId);
}

QVariantList SalesViewController::loadOrders() const {
    QVariantList list;
    const auto orders = m_salesModel->fetchAllOrders();

    for (const auto& o : orders) {
        // 1. Convert the nested C++ items list into a QVariantList
        QVariantList itemsList;
        for (const auto& item : o.items) {
            itemsList.append(QVariantMap{
                {"itemId", item.itemId},
                {"name", item.productName},
                {"quantity", item.quantity},
                {"status", item.serviceState} // Used for filtering in KitchenView
            });
        }

        // 2. Append the full order object, including its nested item model
        list.append(QVariantMap{
            {"orderId", o.id},
            {"tableNumber", o.tableNumber}, // Useful for raw data access
            {"displayTitle", "Table " + o.tableNumber},
            {"status", o.orderStatus},
            {"date", o.createdAt.toString("yyyy-MM-dd hh:mm")},
            {"itemModel", itemsList} // This becomes the model for your inner ListView
        });
    }
    return list;
}

// This returns the "Headers" (Orders) only if they contain items matching the filter
QVariantList SalesViewController::kitchenOrders(const QString& serviceState) const {
    QVariantList filteredList;
    const auto allOrders = m_salesModel->fetchAllOrders();

    for (const auto& order : allOrders) {
        QVariantList itemsList; // This will hold the QML-friendly items
        bool hasMatchingItems = false;

        for (const auto& item : order.items) {
            // Check if item matches current KDS tab (e.g., "ordered")
            if (item.serviceState == serviceState) {
                hasMatchingItems = true;

                // Convert OrderItem struct to QVariantMap
                itemsList.append(QVariantMap{
                    {"itemId", item.itemId},
                    {"name", item.productName},
                    {"quantity", item.quantity},
                    {"status", item.serviceState}
                });
            }
        }

        // Only add the order if it has items for the current state
        if (hasMatchingItems) {
            filteredList.append(QVariantMap{
                {"orderId", order.id},
                {"tableNumber", order.tableNumber},
                {"displayTitle", "Table " + order.tableNumber},
                {"time", order.createdAt.toString("hh:mm")},
                {"itemModel", itemsList} // Now QML can read this!
            });
        }
    }
    return filteredList;
}

// Factory function to create a proxy for a specific card's items
QVariant SalesViewController::createItemProxy(const QVariant& rawItems, const QString& status) {
    // You would typically wrap rawItems in a simple QAbstractListModel first
    // then set it as the source for your GenericFilterProxyModel
    auto* proxy = new GenericFilterProxyModel(this);
    // ... setup source model ...
    proxy->setFilterFixedString(status);
    return QVariant::fromValue(proxy);
}

void SalesViewController::updateAllStatus(const QString& orderId, const QString& status) {
    m_salesModel->updateAllStatus(orderId, status);
}


void SalesViewController::updateItemStatus(const QString& itemId, const QString& status) {
    if (!m_orderItemModel) {
        qWarning() << "OrderItemModel is null!";
        return;
    }

    m_orderItemModel->updateItemStatus(itemId, status);

}

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
    m_productModel(std::make_unique<ProductModel>(this)),
    m_inventoryController(std::make_unique<InventoryViewController>(this)),
    m_filterProxyModel(std::make_unique<ProductFilterProxyModel>(this))
{
    m_filterProxyModel->setSourceModel(m_productModel.get());
    m_filterProxyModel->setCategoryFilter(-1);
    /* Connect using .get() to access the raw pointer owned by unique_ptr */
    connect(m_salesModel.get(), &SalesModel::totalsChanged, this, &SalesViewController::orderChanged);
    connect(m_salesModel.get(), &SalesModel::countChanged, this, &SalesViewController::itemCountChanged);
    connect(m_salesModel.get(), &SalesModel::inventorydbModified,
            m_inventoryController.get(), &InventoryViewController::refresh);
    connect(m_salesModel.get(), &SalesModel::orderStatusChanged,
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

    // ES.71: Prefer range-based for loops for better readability and safety
    for (const auto& o : orders) {
        list.append(QVariantMap{
            {"orderId", o.id},
            {"displayTitle", "Table " + o.tableNumber},
            {"status", o.orderStatus},
            {"date", o.createdAt.toString("yyyy-MM-dd hh:mm")}
        });
    }
    return list;
}

QVariantList SalesViewController::getKitchenQueue() const {
    QVariantList result;
    const auto queue = m_salesModel->fetchKitchenQueue();
    for (const auto& t : queue) {
        result.append(QVariantMap{
            {"orderId", t.orderId},
            {"tableNumber", t.tableNumber},
            {"time", t.timestamp},
            {"items", t.itemsSummary}
        });
    }
    return result;
}

void SalesViewController::updateItemStatus(const QString& orderId, const QString& status) {
    m_salesModel->updateItemStatus(orderId, status);
}

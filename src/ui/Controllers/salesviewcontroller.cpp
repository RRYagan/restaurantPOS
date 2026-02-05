#include "salesviewcontroller.h"
#include "salesmodel.h"
#include "productmodel.h"
#include "inventoryviewcontroller.h"

#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <cashpayment.h>
#include <mpesapayment.h>
#include "payment.h"

// A simple RAII guard to ensure the 'isBusy' flag is reset on function exit
struct BusyGuard {
    bool& target;
    std::function<void()> onExit;
    ~BusyGuard() { target = false; onExit(); }
};

SalesViewController::SalesViewController(QObject* parent)
    : QObject(parent),
    m_salesModel(std::make_unique<SalesModel>(this)),
    m_paymentModel(std::make_unique<PaymentModel>(this)),
    m_orderItemModel(std::make_unique<OrderItemModel>(this)),
    m_productModel(std::make_unique<ProductModel>(this)),
    m_inventoryController(std::make_unique<InventoryViewController>(this)),
    m_genericFilterProxyModel(std::make_unique<GenericFilterProxyModel>(this))
{
    m_genericFilterProxyModel->setSourceModel(m_productModel.get());
    m_genericFilterProxyModel->setCategoryRole(ProductModel::CategoryIdRole);
    m_genericFilterProxyModel->setTypeRole(ProductModel::ProductTypeIdRole);
    loadConfig();

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
    emit kitchenDataChanged();
}


void SalesViewController::updateItemStatus(const QString& itemId, const QString& status) {
    if (!m_orderItemModel) {
        qWarning() << "OrderItemModel is null!";
        return;
    }

    m_orderItemModel->updateItemStatus(itemId, status);
    emit kitchenDataChanged();

}

// payment
void SalesViewController::loadConfig() {
    // 1. Try multiple paths: App Dir, AppData, and Working Dir
    QStringList potentialPaths;
    potentialPaths << QCoreApplication::applicationDirPath() + "/config.json";
    potentialPaths << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
    potentialPaths << "config.json";

    QString finalPath;
    for (const QString &p : potentialPaths) {
        if (QFile::exists(p)) {
            finalPath = p;
            break;
        }
    }

    QFile file(finalPath);
    if (!finalPath.isEmpty() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject mpesa = doc.object().value("mpesa").toObject();

        // 2. Assign values and verify
        m_mpesaConfig.shortCode = mpesa.value("shortCode").toString();
        m_mpesaConfig.passKey = mpesa.value("passKey").toString();
        m_mpesaConfig.consumerKey = mpesa.value("consumerKey").toString();
        m_mpesaConfig.consumerSecret = mpesa.value("consumerSecret").toString();
        m_mpesaConfig.isTill = mpesa.value("isTill").toBool();

        qDebug() << "[Config] Loaded from:" << finalPath;
        qDebug() << "[Config] ShortCode found:" << m_mpesaConfig.shortCode;
    } else {
        qWarning() << "[Config] Could not find config.json in searched paths!";
    }
}

void SalesViewController::startCashPayment() {
    cleanUpActivePayment();

    m_activePayment = new CashPayment(this);

    // --- Step 1: Record the Cash Payment in the DB immediately ---
    PaymentData data;
    data.orderId = m_salesModel->currentOrderId();
    data.type = "CASH";
    data.amount = m_amount;
    data.userTag = "Admin";
    data.externalRef = "CASH-" + data.orderId; // Generate a local reference
    data.status = "Completed";                // Cash is completed instantly

    m_paymentModel->insertPayment(data);

    // --- Step 2: Handle Success & Cleanup ---
    connect(m_activePayment, &Payment::completed, this, [this](const QString &ref) {
        qDebug() << "[SalesView] Cash payment logic finished.";

        this->finalizeTransaction(ref);

        if (m_salesModel) {
            m_salesModel->clear();
        }

        this->resetControllerState();
        m_activePayment->deleteLater();
        m_activePayment = nullptr;

        emit paymentFinished(true, ref);
    });

    connectSignals();
    emit methodNameChanged();

    m_activePayment->process(m_amount, {});
}
void SalesViewController::startMpesaPayment(const QString &phone) {
    cleanUpActivePayment();

    // 1. Initialize the payment object
    m_activePayment = new MpesaPayment(m_mpesaConfig, this);

    // 2. Connect the NEW signals we discussed (see below)
    connectSignals();

    emit methodNameChanged();

    // 3. Prepare data and start process
    QVariantMap data;
    data["phone"] = phone;
    data["order_id"] = m_salesModel->currentOrderId();

    m_activePayment->process(m_amount, data);
}

void SalesViewController::confirmAction() {
    if (m_activePayment) {
        m_activePayment->verifyStatus();
    }
}

void SalesViewController::cancelPayment() {
    if (m_activePayment) {
        m_activePayment->cancel();
    }
}

void SalesViewController::cleanUpActivePayment() {
    if (m_activePayment) {
        m_activePayment->deleteLater();
        m_activePayment = nullptr;
    }
}

void SalesViewController::connectSignals() {
    if (!m_activePayment) return;

    // 1. Cast the pointer so we can access Mpesa-specific signals
    auto mpesa = qobject_cast<MpesaPayment*>(m_activePayment);

    if (mpesa) {
        // --- STEP 1: Capture the initial STK response ---
        // We use 'mpesa' pointer here because 'stkPushInitiated' belongs to MpesaPayment
        connect(mpesa, &MpesaPayment::stkPushInitiated,
                this, [this](const QString &checkoutId, const QString &merchantId) {

                    // Package the data into the struct
                    PaymentData data;
                    data.orderId = m_salesModel->currentOrderId();
                    data.type = "MPESA_STK";
                    data.amount = m_amount; // Assuming m_amount is a Money object
                    data.userTag = "Admin";
                    data.externalRef = checkoutId;
                    data.status = "Awaiting PIN";

                    // Pass the single struct object
                    m_paymentModel->insertPayment(data);
                });

        // --- STEP 2: Capture Polling Results ---
        connect(mpesa, &MpesaPayment::paymentStatusUpdated,
                this, [this](const QString &checkoutId, const QString &status, const QString &receipt) {

                    // 1. Update the Database record
                    m_paymentModel->updatePaymentStatus(checkoutId, status);

                    if (status == "Completed") {
                        qDebug() << "[SalesView] Payment Complete. Running cleanup.";

                        // 2. Finalize logic (Print receipt, log final sale)
                        this->finalizeTransaction(receipt);

                        // 3. Clear the Sales Model (The Cart/Order)
                        if (m_salesModel) {
                            m_salesModel->clear();
                        }
                        this->resetControllerState();
                        // 4. Cleanup the Payment Object memory
                        // We use deleteLater because we are currently inside a signal call from this object
                        m_activePayment->deleteLater();
                        m_activePayment = nullptr;

                        // 5. Notify the UI
                        m_message = "Transaction Successful: " + receipt;
                        emit messageUpdated();
                        emit paymentFinished(true, receipt);
                    }
                });
    }

    // 2. Base Class Connections (Common to all payment types)
    // These use the original m_activePayment (Payment*) pointer
    connect(m_activePayment, &Payment::stateChanged, this, &SalesViewController::stateChanged);

    connect(m_activePayment, &Payment::messageUpdated, this, [this](const QString &msg) {
        m_message = msg;
        emit messageUpdated();
    });

    connect(m_activePayment, &Payment::completed, this, [this](const QString &ref) {
        emit paymentFinished(true, ref);
    });

    connect(m_activePayment, &Payment::errorOccurred, this, [this](const QString &err) {
        m_message = "Error: " + err;
        emit messageUpdated();
    });
}

Payment::State SalesViewController::currentState() const {
    return m_activePayment ? m_activePayment->state() : Payment::State::Idle;
}

QString SalesViewController::currentMessage() const {
    return m_message;
}

QString SalesViewController::currentStateName() const {
    if (!m_activePayment) return QStringLiteral("Idle");

    // Use the enum directly
    Payment::State currentState = m_activePayment->state();
    return m_activePayment->stateToString(static_cast<int>(currentState));
}

void SalesViewController::finalizeTransaction(const QString &receiptNumber) {
    qDebug() << "Finalizing sale with receipt:" << receiptNumber;

    // 1. Emit success to QML
    emit paymentFinished(true, receiptNumber);

    // 2. Here you would usually call your sales model to clear the cart
    // m_salesModel->clearCart();

    // 3. Perhaps trigger a receipt print
    // ReceiptPrinter::instance().print(m_currentOrderId);
}


// todo: refactor
void SalesViewController::resetControllerState() {
    // 1. Reset the Money object
    m_amount = Money(0); // This creates a new Money struct with 0 cents

    // OR if you don't have a constructor that takes 0:
    // m_amount.cents = 0;

    // 2. Clear the UI message
    m_message = "";

    // 3. Notify the UI (QML) that the amount/total has changed
    // Assuming you have a signal like amountChanged()
    emit amountChanged();
    emit messageUpdated();

    qDebug() << "[SalesView] Controller state and m_amount cleared.";
}

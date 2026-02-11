#include "salesviewcontroller.h"
#include "salesmodel.h"
#include "productmodel.h"
#include "inventoryviewcontroller.h"

#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>
#include <cashpayment.h>
#include <mpesapayment.h>
#include "payment.h"
#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>


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


    connectSignals();
    // emit methodNameChanged();

    m_activePayment->process(m_amount, {});
}

void SalesViewController::startMpesaPayment(const QString &phone) {
    // 1. Clear old data and force state back to Idle
    cleanUpActivePayment();
    emit stateChanged(); // Tells QML: "We are now Idle/Reset"

    // 2. Create the new payment
    m_activePayment = new MpesaPayment(m_mpesaConfig, this);

    // 3. Connect signals so we hear when it moves to Initiated/Success
    connectSignals();

    // 4. Set local message to "Starting..."
    m_message = "Initializing M-Pesa...";
    emit messageUpdated();

    // 5. Actually start the process
    m_activePayment->process(m_amount, {{"phone", phone}});
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

                    m_paymentModel->updatePaymentStatus(checkoutId, status);

                    if (status == "Completed") {
                        qDebug() << "[SalesView] Payment Complete. Running cleanup.";

                        this->finalizeTransaction(receipt);

                        if (m_salesModel) {
                            m_salesModel->clear();
                        }

                        // --- THE FIX IS HERE ---
                        // Calling this function handles the deleteLater() AND the nullptr assignment.
                        // Do NOT touch m_activePayment again after this line.
                        this->resetControllerState();

                        m_message = "Transaction Successful: " + receipt;
                        emit messageUpdated();
                        emit paymentFinished(true, receipt);
                    }
                });
    }


    connect(m_activePayment, &Payment::stateChanged, this, [this]() {
        emit stateChanged(); // Keep UI in sync

        PaymentStatus::State s = m_activePayment->state();

        if (s == PaymentStatus::Failed || s == PaymentStatus::Cancelled) {
            qDebug() << "[Production] Transaction stopped with status:" << s;

            // We use a small delay (e.g., 3-5 seconds) so the user
            // can actually read the error message on the screen.
            QTimer::singleShot(5000, this, &SalesViewController::resetControllerState);
        }
    });

    connect(m_activePayment, &Payment::messageUpdated, this, [this](const QString &msg) {
        m_message = msg;
        emit messageUpdated();
    });

    connect(m_activePayment, &Payment::completed, this, [this](const QString &ref) {
        // 1. Finalize the DB/UI logic
        this->finalizeTransaction(ref);

        // 2. Do NOT manually delete here.
        // Let the next payment start cycle handle the cleanup
        // or use a single shot timer if you must.
        // QTimer::singleShot(0, this, [this]() {
        //     this->cleanUpActivePayment();
        // });
    });

    connect(m_activePayment, &Payment::errorOccurred, this, [this](const QString &err) {
        m_message = "Error: " + err;
        emit messageUpdated();
        emit stateChanged();
    });
}


PaymentStatus::State SalesViewController::currentState() const {
    // If no payment object exists, we are Idle
    if (!m_activePayment) return PaymentStatus::State::Idle;

    // Pull the real-time state from the Mpesa/Cash object
    return m_activePayment->state();
}
QString SalesViewController::currentMessage() const {
    return m_message;
}

QString SalesViewController::currentStateName() const {
    if (!m_activePayment) return "Idle";

    switch (m_activePayment->state()) {
    case PaymentStatus::State::Idle:           return "Idle";
    case PaymentStatus::State::Initiated:      return "Initiated";
    case PaymentStatus::State::AwaitingAction: return "AwaitingAction";
    case PaymentStatus::State::Verifying:      return "Verifying";
    case PaymentStatus::State::Success:        return "Success";
    case PaymentStatus::State::Failed:         return "Failed";
    case PaymentStatus::State::Cancelled:      return "Cancelled";
    default:                             return "Unknown";
    }
}


void SalesViewController::finalizeTransaction(const QString &receiptNumber) {
    qDebug() << "[Database] Finalizing sale record...";

    if (!m_salesModel) return;

    // 1. Prepare Data
    // Fix: Use QUuid::StringFormat::WithoutBrackets
    QString saleId = QUuid::createUuid().toString();
    QString orderId = m_salesModel->currentOrderId();
    QString userId = "1"; // Replace with your actual user session ID
    QString dateStr = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Financials from SalesModel (using your existing double logic)
    double total = m_salesModel->totalAmount();
    double tax = m_salesModel->totalTaxAmount();
    double net = total - tax;
    double gross = total - tax; // Sub-total + surcharge (before tax)

    // Map payment method based on your SQL INSERT list:
    // CASH = 1 (code 01), MOBILE MONEY = 6 (code 06)
    int methodId = (methodName() == "MPESA") ? 6 : 1;

    // 2. Database Operation
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        qWarning() << "Could not start transaction";
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO sale (
            id, order_id, user_id, sale_transaction_date,
            net_amount, gross_amount, tax_amount, total_amount,
            kra_receipt_number, payment_method_id
        ) VALUES (
            :id, :order_id, :user_id, :date,
            :net, :gross, :tax, :total,
            :receipt, :method
        )
    )");

    query.bindValue(":id", saleId);
    query.bindValue(":order_id", orderId);
    query.bindValue(":user_id", userId);
    query.bindValue(":date", dateStr);
    query.bindValue(":net", net);
    query.bindValue(":gross", gross);
    query.bindValue(":tax", tax);
    query.bindValue(":total", total);
    query.bindValue(":receipt", receiptNumber);
    query.bindValue(":method", methodId);

    if (!query.exec()) {
        qCritical() << "[Database] Sale Insert Failed:" << query.lastError().text();
        db.rollback();
        emit paymentFinished(false, "Database Save Failed");
        return;
    }

    if (db.commit()) {
        qDebug() << "[Database] Sale saved successfully. ID:" << saleId;

        // 3. UI/State Cleanup
        emit paymentFinished(true, receiptNumber);
        resetControllerState(); // Clears cart and resets payment objects
    } else {
        db.rollback();
    }
}
// todo: refactor
void SalesViewController::resetControllerState() {
    if (!m_activePayment) return;

    // 1. Isolate the payment object
    m_activePayment->disconnect(this);
    auto oldPayment = m_activePayment;
    m_activePayment = nullptr;
    oldPayment->deleteLater();

    // 2. Clear Order Data
    m_amount = Money(0);
    m_message = "";

    if (m_salesModel) {
        m_salesModel->clear();
    }

    // 4. Update UI Bindings
    emit amountChanged();
    emit messageUpdated();
    emit stateChanged();
    // emit orderIdChanged(); // Notify QML that the order reference is gone

    qDebug() << "[Production] Order and Payment state cleared completely.";
}

#include "paymentcontroller.h"

PaymentController::PaymentController(QObject *parent) : QObject(parent) {
    // Initialize your M-Pesa credentials here or load from DB
    m_mpesaConfig.shortCode = "174379";
    m_mpesaConfig.passKey = "bfb279f9aa9bdbcf158e97dd71a467cd2e0c893059b10f78e6b72ada1ed2c919";
    m_mpesaConfig.consumerKey = "YOUR_KEY";
    m_mpesaConfig.consumerSecret = "YOUR_SECRET";
}

PaymentController::~PaymentController() {
    cleanUpActivePayment();
}

void PaymentController::startCashPayment() {
    cleanUpActivePayment();
    m_activePayment = new CashPayment(this);
    connectSignals();
    m_activePayment->process(m_amount, {});
}

void PaymentController::startMpesaPayment(const QString &phone) {
    cleanUpActivePayment();
    m_activePayment = new MpesaPayment(m_mpesaConfig, this);
    connectSignals();

    QVariantMap data;
    data["phone"] = phone;
    m_activePayment->process(m_amount, data);
}

void PaymentController::confirmAction() {
    if (m_activePayment) {
        m_activePayment->verifyStatus();
    }
}

void PaymentController::cancelPayment() {
    if (m_activePayment) {
        m_activePayment->cancel();
    }
}

void PaymentController::cleanUpActivePayment() {
    if (m_activePayment) {
        m_activePayment->deleteLater();
        m_activePayment = nullptr;
    }
}

void PaymentController::connectSignals() {
    if (!m_activePayment) return;

    connect(m_activePayment, &Payment::stateChanged, this, &PaymentController::stateChanged);

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

Payment::State PaymentController::currentState() const {
    return m_activePayment ? m_activePayment->state() : Payment::State::Idle;
}

QString PaymentController::currentMessage() const {
    return m_message;
}

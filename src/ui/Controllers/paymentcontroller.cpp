#include "paymentcontroller.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <paymentmodel.h>


PaymentController::PaymentController(QObject *parent) : QObject(parent) {
    loadConfig();
}

void PaymentController::loadConfig() {
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

    PaymentModel model;
    int amountCents = static_cast<int>(m_amount * 100);

    // We log it as 'Initiated' with the current user tag
    model.insertPayment("ORD-TEMP-123", "MPESA_STK", amountCents, "Admin", "");

    m_activePayment = new MpesaPayment(m_mpesaConfig, this);
    connectSignals();

    if (isTokenValid()) {
        qDebug() << "[M-Pesa] Using Cached Token. Skipping Handshake.";
        // You would add a setter in MpesaPayment for this
        // m_activePayment->setToken(m_cachedToken);
    }

    QVariantMap data;
    data["phone"] = phone;
    data["user_tag"] = "Admin";
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

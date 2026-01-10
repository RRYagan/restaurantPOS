#include "paymentmodel.h"
#include <QUuid>

PaymentModel::PaymentModel(QObject *parent) : QObject(parent) {}

bool PaymentModel::insertPayment(const QString &orderId,
                                 const QString &type,
                                 int amountCents,
                                 const QString &userTag,
                                 const QString &externalRef)
{
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) {
        qCritical() << "PaymentModel: Database not open";
        return false;
    }

    QSqlQuery query(db);
    // Matches your schema in databasemanager.cpp
    query.prepare("INSERT INTO payments ("
                  "id, order_id, payment_type, amount_cents, status, user_tag, external_reference"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?)");

    QString internalId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.addBindValue(internalId);
    query.addBindValue(orderId);
    query.addBindValue(type);
    query.addBindValue(amountCents);
    query.addBindValue("Initiated"); // Default starting status
    query.addBindValue(userTag);
    query.addBindValue(externalRef);

    if (!query.exec()) {
        qCritical() << "PaymentModel Error:" << query.lastError().text();
        emit paymentLogged(orderId, false);
        return false;
    }

    qDebug() << "[DB] Payment recorded for Order:" << orderId << "Type:" << type;
    emit paymentLogged(orderId, true);
    return true;
}

bool PaymentModel::updatePaymentStatus(const QString &externalRef, const QString &newStatus)
{
    QSqlDatabase db = DatabaseManager::instance().database();
    QSqlQuery query(db);

    query.prepare("UPDATE payments SET status = ? WHERE external_reference = ?");
    query.addBindValue(newStatus);
    query.addBindValue(externalRef);

    return query.exec();
}

// paymentmodel.cpp
#include <QSettings>

void PaymentModel::cacheToken(const QString &token, int expiresInSeconds) {
    QSettings settings;
    // Buffer 60 seconds to be safe
    QDateTime expiry = QDateTime::currentDateTime().addSecs(expiresInSeconds - 60);

    settings.setValue("mpesa/token", token);
    settings.setValue("mpesa/expiry", expiry.toString(Qt::ISODate));
    settings.sync(); // Force write to file
    qDebug() << "[Cache] Token saved to file. Expires:" << expiry.toString();
}

QString PaymentModel::getValidToken() {
    QSettings settings;
    QString token = settings.value("mpesa/token").toString();
    QString expiryStr = settings.value("mpesa/expiry").toString();

    if (token.isEmpty() || expiryStr.isEmpty()) return QString();

    QDateTime expiry = QDateTime::fromString(expiryStr, Qt::ISODate);
    if (QDateTime::currentDateTime() < expiry) {
        qDebug() << "[Cache] Valid token found in file.";
        return token;
    }

    return QString(); // Expired or missing
}

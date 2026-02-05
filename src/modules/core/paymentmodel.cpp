#include "money.h"
#include "paymentmodel.h"
#include <QUuid>
#include <QSettings>

PaymentModel::PaymentModel(QObject *parent) : QObject(parent) {}

bool PaymentModel::insertPayment(const PaymentData &data) {
    QSqlQuery query;

    // R"sql(...)sql" allows you to write clean SQL exactly as it looks in a DB editor
    query.prepare(R"sql(
        INSERT INTO payments (
            id,
            order_id,
            payment_type,
            amount_cents,
            status,
            user_tag,
            external_reference
        )
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )sql");

    QString internalId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.addBindValue(internalId);                                // 1. id
    query.addBindValue(data.orderId);                              // 2. order_id
    query.addBindValue(data.type);                                 // 3. payment_type
    query.addBindValue(static_cast<qlonglong>(data.amount.cents));  // 4. amount_cents
    query.addBindValue(data.status);                               // 5. status
    query.addBindValue(data.userTag);                              // 6. user_tag
    query.addBindValue(data.externalRef);                          // 7. external_reference

    if (!query.exec()) {
        qCritical() << "--- Database Error ---";
        qCritical() << "Error Text   :" << query.lastError().text();
        qCritical() << "Executed SQL  :" << query.executedQuery();

        QVariantList list = query.boundValues();
        for (int i = 0; i < list.count(); ++i) {
            qCritical() << QString("  Index %1: %2").arg(i).arg(list.at(i).toString());
        }

        emit paymentLogged(data.orderId, false);
        return false;
    }

    qDebug() << "[DB] Payment recorded for Order:" << data.orderId << "Type:" << data.type;
    emit paymentLogged(data.orderId, true);
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

bool PaymentModel::updateTransactionId(const QString &orderId, const QString &transactionId)
{
    QSqlQuery query;
    query.prepare("UPDATE payments SET transaction_id = ? WHERE order_id = ?");
    query.addBindValue(transactionId);
    query.addBindValue(orderId);

    if (!query.exec()) {
        qCritical() << "Failed to update Transaction ID:" << query.lastError().text();
        return false;
    }
    return true;
}
void PaymentModel::cacheToken(const QString &token, int expiresInSeconds) {
    QSettings settings;
    // Buffer 60 seconds to be safe
    QDateTime expiry = QDateTime::currentDateTime().addSecs(expiresInSeconds - 60);

    settings.setValue("mpesa/token", token);
    settings.setValue("mpesa/expiry", expiry.toString(Qt::ISODate));
    settings.sync(); // Force write to file
    qDebug() << "[Cache] Token saved to file. Expires:" << expiry.toString();
}

// In paymentmodel.cpp
QString PaymentModel::getValidToken() {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT access_token FROM oauth_tokens "
                  "WHERE provider = 'mpesa' AND expiry_time > DATETIME('now')");

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

void PaymentModel::saveMpesaToken(const QString &token, int expiresIn) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT OR REPLACE INTO oauth_tokens (provider, access_token, expiry_time) "
                  "VALUES ('mpesa', ?, DATETIME('now', ? || ' seconds'))");
    query.addBindValue(token);
    query.addBindValue(expiresIn);
    query.exec();
}

// In paymentmodel.cpp
bool PaymentModel::clearAllPayments() {
    QSqlDatabase db = DatabaseManager::instance().database();
        QSqlQuery query(db);

        // Use a transaction for safety as seen in your DatabaseManager
        if (!db.transaction()) return false;

    if (!query.exec("DELETE FROM payments")) {
        qCritical() << "Failed to clear payments:" << query.lastError().text();
            db.rollback();
    }

    // Reset sqlite sequence for IDs if necessary
    query.exec("DELETE FROM sqlite_sequence WHERE name='payments'");

    return db.commit();
}

#include "usermodel.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QUuid>
#include <QCryptographicHash>

bool UserModel::addUser(const QString &username, const QString &password, const QString &role) {
    QString salt = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    QString hash = hashPassword(password, salt);
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("INSERT INTO users (username, password_hash, salt, role) VALUES (?, ?, ?, ?)");
    q.addBindValue(username); q.addBindValue(hash); q.addBindValue(salt); q.addBindValue(role);
    return q.exec();
}

bool UserModel::verifyUser(const QString &username, const QString &password) {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT id, password_hash, salt, role FROM users WHERE username = ?");
    q.addBindValue(username);
    if (q.exec() && q.next()) {
        if (hashPassword(password, q.value(2).toString()) == q.value(1).toString()) {
            m_session.userId = q.value(0).toInt();
            m_session.username = username;
            m_session.role = q.value(3).toString();
            m_session.isValid = true;
            return true;
        }
    }
    m_session = UserSession();
    return false;
}

QString UserModel::hashPassword(const QString& password, const QString& salt) {
    QByteArray hash = QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}
bool UserModel::deleteUser(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM users WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}


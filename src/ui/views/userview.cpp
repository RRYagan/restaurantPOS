#include "userview.h"
#include "databasemanager.h"
#include <QSqlQuery>

UserView::UserView(QObject *parent) : QAbstractListModel(parent) {
    refresh();
}

int UserView::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_users.size();
}

QVariant UserView::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_users.size()) return QVariant();
    const auto &user = m_users[index.row()];
    if (role == UsernameRole) return user.username;
    if (role == RoleRole) return user.role;
    if (role == IdRole) return user.id;
    return QVariant();
}

QHash<int, QByteArray> UserView::roleNames() const {
    return { {UsernameRole, "username"}, {RoleRole, "role"}, {IdRole, "id"} };
}

void UserView::refresh() {
    beginResetModel();
    m_users.clear();
    QSqlQuery query("SELECT id, username, role FROM users");
    while (query.next()) {
        m_users.append({query.value(0).toInt(), query.value(1).toString(), query.value(2).toString()});
    }
    endResetModel();
}

// Call DatabaseManager methods
bool UserView::addUser(const QString &user, const QString &pass, const QString &role) {
    if (DatabaseManager::instance().addUser(user, pass, role)) {
        refresh();
        return true;
    }
    return false;
}

// userview.cpp

bool UserView::isLoggedIn() const {
    return DatabaseManager::instance().currentUser().isValid;
}

QString UserView::currentUserRole() const {
    return DatabaseManager::instance().currentUser().role;
}
bool UserView::isAdmin() const {
    return DatabaseManager::instance().isAdmin();
}
bool UserView::login(const QString &username, const QString &password) {
    if (DatabaseManager::instance().verifyUser(username, password)) {
        emit sessionChanged();
        return true;
    }
    return false;
}

void UserView::logout() {
    DatabaseManager::instance().logout();
    emit sessionChanged();
}

bool UserView::deleteUser(int id) {
    // Session check: Only managers can delete users
    if (currentUserRole() != "manager") {
        qDebug() << "Unauthorized: Manager role required to delete users.";
        return false;
    }

    if (DatabaseManager::instance().deleteUser(id)) {
        refresh();
        return true;
    }
    return false;
}

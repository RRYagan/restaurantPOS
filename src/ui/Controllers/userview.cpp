// #include "usermodel.h"
// #include "databasemanager.h"
// #include <QSqlQuery>
// #include <QSqlError>
// #include <QDebug>

// bool UserModel::createUser(const User &user) {
//     QSqlQuery query(DatabaseManager::instance().database());
//     query.prepare("INSERT INTO users (staff_id_number, full_name, user_role, login_username, password_hash) "
//                   "VALUES (?, ?, ?, ?, ?)");
//     query.addBindValue(user.staffIdNumber);
//     query.addBindValue(user.fullName);
//     query.addBindValue(user.userRole);
//     query.addBindValue(user.loginUsername);
//     query.addBindValue(user.passwordHash);

//     if (!query.exec()) {
//         qCritical() << "Create user failed:" << query.lastError().text();
//         return false;
//     }
//     return true;
// }

// User UserModel::getUserByUsername(const QString &username) {
//     User user;
//     user.id = -1;
//     QSqlQuery query(DatabaseManager::instance().database());
//     query.prepare("SELECT id, staff_id_number, full_name, user_role, login_username, password_hash, is_active_status "
//                   "FROM users WHERE login_username = ?");
//     query.addBindValue(username);

//     if (query.exec() && query.next()) {
//         user.id = query.value(0).toInt();
//         user.staffIdNumber = query.value(1).toString();
//         user.fullName = query.value(2).toString();
//         user.userRole = query.value(3).toString();
//         user.loginUsername = query.value(4).toString();
//         user.passwordHash = query.value(5).toString();
//         user.isActive = (query.value(6).toString() == "Y");
//     }
//     return user;
// }

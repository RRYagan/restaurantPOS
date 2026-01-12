#ifndef USERMODEL_H
#define USERMODEL_H

#include <QString>
#include <QVariantList>

struct User {
    int id;
    QString staffIdNumber;
    QString fullName;
    QString userRole;
    QString loginUsername;
    QString passwordHash;
    bool isActive;
};

class UserModel {
public:
    static bool createUser(const User &user);
    static User getUserById(int id);
    static User getUserByUsername(const QString &username);
    static bool updateUser(const User &user);
    static bool deleteUser(int id);
    static QList<User> getAllActiveUsers();
};

#endif

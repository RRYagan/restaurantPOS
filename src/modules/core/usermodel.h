#ifndef USERMODEL_H
#define USERMODEL_H

#include <QString>
#include <QVariantList>
#include "types.h"

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

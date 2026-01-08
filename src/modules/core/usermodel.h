#ifndef USERMODEL_H
#define USERMODEL_H

#include <QObject>
#include "usersession.h"

class UserModel : public QObject {
    Q_OBJECT
public:
    bool addUser(const QString &username, const QString &password, const QString &role);
    bool verifyUser(const QString &username, const QString &password);
    bool deleteUser(int id);
    bool updateUser(int id, const QString &username, const QString &role);


    UserSession currentUser() const { return m_session; }
    void logout() { m_session = UserSession(); }
    bool isAdmin() const { return m_session.role == "manager"; }

private:
    QString hashPassword(const QString& password, const QString& salt);
    UserSession m_session;
};
#endif

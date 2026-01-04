#ifndef USERSESSION_H
#define USERSESSION_H

struct UserSession {
    int userId;
    QString username;
    QString role;
    bool isValid = false;
};


#endif // USERSESSION_H

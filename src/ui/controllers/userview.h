#ifndef USERVIEW_H
#define USERVIEW_H

#include <QAbstractListModel>
#include <user.h>
#include <usermodel.h>
#include <QtQml/qqmlregistration.h>

class UserView : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY sessionChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY sessionChanged)

public:
    enum UserRoles { IdRole = Qt::UserRole + 1, UsernameRole, RoleRole };
    explicit UserView(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isLoggedIn() const;
    QString currentUserRole() const;
    bool isAdmin() const;

    Q_INVOKABLE bool login(const QString &username, const QString &password);
    Q_INVOKABLE void logout();

    Q_INVOKABLE bool addUser(const QString &user, const QString &pass, const QString &role);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool deleteUser(int id);

signals:
    void sessionChanged();
private:
    QVector<User> m_users;
    UserModel m_model;
};

#endif

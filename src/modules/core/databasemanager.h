#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "inventoryitem.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <menuitem.h>
#include <order.h>
#include <usersession.h>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

    bool openDatabase(const QString& path = QString());
    void closeDatabase();

    bool saveOrder(Order& order);

    QVector<MenuItem> fetchMenuItems(const QString &categoryFilter);
    QStringList fetchCategories();

    // menu
    QVariantList getAllMenuItems();
    bool addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon);
    bool updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon);
    bool deleteMenuItem(int id);

    int generateOrderId();
    Order loadOrder(const QString& orderId);

    // category
    bool deleteCategory(const QString& categoryName);
    bool addCategory(const QString& name);
    bool updateCategory(const QString& oldName, const QString& newName);


    // Users
    bool addUser(const QString &username, const QString &password, const QString &role);
    bool updateUser(int id, const QString &username, const QString &role);
    bool deleteUser(int id);
    bool verifyUser(const QString &username, const QString &password);
    UserSession currentUser() const { return m_session; }
    void logout() { m_session = UserSession(); }
    bool isAdmin() const { return m_session.role == "manager"; }

    // Inventory
    bool addInventoryItem(const QString &name, int quantity, const QString &unit);
    bool updateInventoryItem(int id, const QString &name, int quantity, const QString &unit);
    bool deleteInventoryItem(int id);
    QVariantList fetchInventory(); // Returns a list of maps or a custom struct

    //inventory history
    QVariantList fetchAllInventoryHistory();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initSchema();
    void seedDatabase();
    QSqlDatabase m_db;
    UserSession m_session;
    QString hashPassword(const QString& password, const QString& salt); // New helper
};

#endif

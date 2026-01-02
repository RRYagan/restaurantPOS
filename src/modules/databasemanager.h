#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "order.h"
#include "menuitem.h"

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

    bool saveOrder(const Order& order);
    // Added implementations for these based on your requirements
    QList<MenuItem> getAllMenuItems();
    int generateOrderId();
    Order loadOrder(int orderId);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initSchema();
    void seedDatabase();

    QSqlDatabase m_db;
};

#endif

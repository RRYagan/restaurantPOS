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

    bool saveOrder(Order& order);

    QVector<MenuItem> fetchMenuItems(const QString &categoryFilter);
    QStringList fetchCategories();

    // menu
    QList<MenuItem> getAllMenuItems();
    bool addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon);
    bool updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon);
    bool deleteMenuItem(int id);

    int generateOrderId();
    Order loadOrder(const QString& orderId);

    // category
    bool deleteCategory(const QString& categoryName);
    bool addCategory(const QString& name);
    bool updateCategory(const QString& oldName, const QString& newName);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initSchema();
    void seedDatabase();

    QSqlDatabase m_db;
};

#endif

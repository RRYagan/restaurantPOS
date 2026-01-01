#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#pragma once
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

    bool openDatabase();
    void closeDatabase();
    void seedDatabase();

    bool saveOrder(const Order& order);
    Order loadOrder(int orderId);
    QList<MenuItem> getAllMenuItem();
    int generateOrderId();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initSchema();
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H

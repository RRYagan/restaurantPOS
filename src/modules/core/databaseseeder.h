#pragma once

#include <QSqlDatabase>

class DatabaseSeeder
{
public:
    explicit DatabaseSeeder(QSqlDatabase db);

    void seedIfNeeded();
    void forceSeed();

private:
    bool seedUsers();
    bool seedProducts();
    bool seedProductCompositions();
    bool seedOpeningStockMovements();

    bool isSeeded() const;
    bool markSeeded() const;

private:
    QSqlDatabase m_db;
};

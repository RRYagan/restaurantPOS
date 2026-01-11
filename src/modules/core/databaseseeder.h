#ifndef DATABASESEEDER_H
#define DATABASESEEDER_H

#include <QSqlDatabase>

class DatabaseSeeder
{
public:
    explicit DatabaseSeeder(QSqlDatabase db);

    void seedIfNeeded();
    void forceSeed(); // for tests

private:
    bool isSeeded() const;
    bool markSeeded() const;

    bool seedUsers() const;
    bool seedProducts() const;
    bool seedMenu() const;
    bool isTableEmpty(const QString& tableName);

    QSqlDatabase m_db;
};

#endif // DATABASESEEDER_H

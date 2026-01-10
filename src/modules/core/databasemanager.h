#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <databaseseeder.h>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

    bool openDatabase(const QString& path = QString());
    void closeDatabase();
    QSqlDatabase database() const { return m_db; }

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initSchema();
    QSqlDatabase m_db;
    DatabaseSeeder seeder;
};

#endif

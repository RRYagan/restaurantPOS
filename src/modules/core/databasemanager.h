#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

    // Open database file
    bool openDatabase(const QString& path = QString(), bool forceSeed=false);
    void closeDatabase();
    QSqlDatabase database() const { return m_db; }

    // ----------------------------
    // New functions
    // ----------------------------
    bool initializeIfNew();           // Run initial schema if DB is new
    bool runPendingMigrations();      // Apply migrations not yet applied

private:
    explicit DatabaseManager(QObject *parent = nullptr);

    bool initSchemaFromResource(const QString& sqlResourcePath);
    bool initMigrationSchema();       // Create schema_migrations table if missing
    bool applyMigration(const QString& sqlResourcePath); // Apply single migration

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_isNewDatabase = false;
    bool forceSeed = false;
};

#endif

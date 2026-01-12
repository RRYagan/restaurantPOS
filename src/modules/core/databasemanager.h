#pragma once

#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool openDatabase(const QString& path = QString(), bool forceSeed = false);
    void closeDatabase();

    QSqlDatabase database() const { return m_db; }

private:
    explicit DatabaseManager(QObject* parent = nullptr);

    bool initMigrationSchema();
    bool initializeIfNew();
    bool runPendingMigrations();
    bool applyMigration(const QString& resourcePath);
    bool executeSqlResource(const QString& resourcePath);

private:
    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_isNewDatabase = false;
};

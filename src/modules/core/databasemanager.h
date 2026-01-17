#pragma once

#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // Singleton access using modern trailing return type
    static auto instance() -> DatabaseManager&;

    // Rule of Five: Explicitly delete copy and move operations
    DatabaseManager(const DatabaseManager&) = delete;
    auto operator=(const DatabaseManager&) -> DatabaseManager& = delete;
    DatabaseManager(DatabaseManager&&) = delete;
    auto operator=(DatabaseManager&&) -> DatabaseManager& = delete;

    // Define a default destructor
    ~DatabaseManager() override = default;

    auto openDatabase(const QString& path = QString(), bool forceSeed = false) -> bool;
    auto closeDatabase() -> void;

    [[nodiscard]] auto database() const -> QSqlDatabase { return m_db; }

private:
    explicit DatabaseManager(QObject* parent = nullptr);


    auto initMigrationSchema() -> bool;
    auto initializeIfNew() -> bool;
    auto runPendingMigrations() -> bool;
    auto applyMigration(const QString& resourcePath) -> bool;
    auto executeSqlResource(const QString& resourcePath) -> bool;

private:
    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_isNewDatabase = false;
};

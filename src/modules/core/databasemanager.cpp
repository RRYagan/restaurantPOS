#include "databasemanager.h"
#include "databaseseeder.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <QSet>
#include <algorithm>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{}

auto DatabaseManager::instance() -> DatabaseManager&
{
    static DatabaseManager instance;
    return instance;
}

auto DatabaseManager::openDatabase(const QString& path, bool forceSeed) -> bool
{
    m_dbPath = path;
    if (m_dbPath.isEmpty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_dbPath = dir + "/restaurant.db";
    }

    m_isNewDatabase = !QFile::exists(m_dbPath);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qCritical() << "DB open failed:" << m_db.lastError().text();
        return false;
    }

    if (!initMigrationSchema())
    {
        return false;
    }

    if (m_isNewDatabase) {
        if (!initializeIfNew()){
            return false;
        }
    }

    if (!runPendingMigrations()){
        return false;
    }

    // Note: Ensure DatabaseSeeder constructor accepts const QSqlDatabase&
    // to avoid performance-unnecessary-value-param warnings.
    DatabaseSeeder seeder(m_db);
    forceSeed ? seeder.forceSeed() : seeder.seedIfNeeded();

    return true;
}

auto DatabaseManager::closeDatabase() -> void
{
    QString name = m_db.connectionName();
    if (m_db.isOpen())
    {
        m_db.close();

    }
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

auto DatabaseManager::initMigrationSchema() -> bool
{
    QSqlQuery q(m_db);
    return q.exec(R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version TEXT PRIMARY KEY,
            applied_at DATETIME NOT NULL
        )
    )");
}

auto DatabaseManager::initializeIfNew() -> bool
{
    qDebug() << "Initializing new database schema...";
    return executeSqlResource(":/sql/migrations/000_init.up.sql");
}

auto DatabaseManager::runPendingMigrations() -> bool
{
    QSqlQuery q(m_db);
    if (!q.exec("SELECT version FROM schema_migrations")) {
        qCritical() << q.lastError().text();
        return false;
    }

    QSet<QString> applied;
    while (q.next()){
        applied.insert(q.value(0).toString());
    }

    QStringList migrations;
    QDirIterator it(":/sql/migrations", QStringList() << "*.up.sql", QDir::Files);
    while (it.hasNext())
    {
        migrations << it.next();
    }

    std::sort(migrations.begin(), migrations.end());

    for (const QString& path : std::as_const(migrations)) {
        QString version = QFileInfo(path).fileName().section('.', 0, 0);
        if (applied.contains(version))
        { continue; }

        qDebug() << "Applying migration:" << version;
        if (!applyMigration(path))
        {
            return false;
        }
    }
    return true;
}

auto DatabaseManager::applyMigration(const QString& resourcePath) -> bool
{
    if (!executeSqlResource(resourcePath)) { return false; }

    QString version = QFileInfo(resourcePath).fileName().section('.', 0, 0);
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO schema_migrations VALUES (?, CURRENT_TIMESTAMP)");
    q.addBindValue(version);

    if (!q.exec()) {
        qCritical() << "Failed recording migration:" << q.lastError().text();
        return false;
    }
    return true;
}

auto DatabaseManager::executeSqlResource(const QString& resourcePath) -> bool
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open SQL:" << resourcePath;
        return false;
    }

    QString sql = QTextStream(&file).readAll();
    file.close();

    QStringList statements;
    QString current;
    bool inTrigger = false;

    for (const QString& line : sql.split('\n')) {
        QString trimmed = line.trimmed();

        if (trimmed.startsWith("CREATE TRIGGER", Qt::CaseInsensitive)) {
            inTrigger = true;
        }

        current += line + "\n";

        if (!inTrigger && trimmed.endsWith(";")) {
            statements << current.trimmed();
            current.clear();
        }

        if (inTrigger && trimmed == "END;") {
            statements << current.trimmed();
            current.clear();
            inTrigger = false;
        }
    }

    if (!current.trimmed().isEmpty())
    {
        statements << current.trimmed();
    }

    QSqlQuery q(m_db);

    if (!m_db.transaction()) {
        qCritical() << "Failed to start transaction";
        return false;
    }

    for (const QString& stmt : std::as_const(statements)) {
        if (stmt.isEmpty()){
            continue;
        }

        if (!q.exec(stmt)) {
            qCritical() << "SQL failed:\n"
                        << stmt << "\nError:"
                        << q.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

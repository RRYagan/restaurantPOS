#include "databasemanager.h"
#include "databaseseeder.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager _instance;
    return _instance;
}


bool DatabaseManager::openDatabase(const QString& path, bool forceSeed)
{
    m_dbPath = path;

    if (m_dbPath.isEmpty()) {
        QString dataPath =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataPath);
        m_dbPath = dataPath + "/restaurant.db";
    }

    qDebug() << "Database path:" << m_dbPath;

    m_isNewDatabase = !QFile::exists(m_dbPath);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // 1. Ensure migration tracking table exists
    if (!initMigrationSchema()) {
        qCritical() << "Failed to create schema_migrations table";
        return false;
    }

    // 2. Initialize base schema ONLY if DB file is new
    if (m_isNewDatabase) {
        if (!initializeIfNew()) {
            qCritical() << "Failed to initialize new database schema";
            return false;
        }
    }

    // 3. Apply pending migrations (always safe)
    if (!runPendingMigrations()) {
        qCritical() << "Failed to apply pending migrations";
        return false;
    }

    // 4. Seed logic
    DatabaseSeeder seeder(m_db);

    if (forceSeed) {
        qWarning() << "Force seeding database";
        seeder.forceSeed();
    } else {
        seeder.seedIfNeeded();
    }

    return true;
}


void DatabaseManager::closeDatabase()
{
    QString connName = m_db.connectionName();
    if (m_db.isOpen()) m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connName);
}

// ----------------------------
// Initialize initial schema from resource
// ----------------------------
bool DatabaseManager::initializeIfNew()
{
    qDebug() << "Initializing new database schema...";
    if (!initSchemaFromResource(":/sql/migrations/000_init.up.sql")) {
        qCritical() << "Failed to initialize initial schema";
        return false;
    }

    // Record that this migration has been applied
    QSqlQuery query(m_db);
    if (!query.exec("INSERT INTO schema_migrations (version, applied_at) "
                    "VALUES ('000_init', CURRENT_TIMESTAMP)")) {
        qCritical() << "Failed to record initial migration:" << query.lastError().text();
        return false;
    }

    qDebug() << "Initial schema applied successfully.";
    return true;
}

// ----------------------------
// Apply all pending migrations
// ----------------------------
bool DatabaseManager::runPendingMigrations()
{
    QSqlQuery query(m_db);
    if (!query.exec("SELECT version FROM schema_migrations")) {
        qCritical() << "Failed to query applied migrations:" << query.lastError().text();
        return false;
    }

    QSet<QString> applied;
    while (query.next()) {
        applied.insert(query.value(0).toString());
    }

    QDirIterator it(":/sql/migrations", QStringList() << "*.sql",
                    QDir::Files, QDirIterator::NoIteratorFlags);

    bool success = true;

    while (it.hasNext()) {
        QString path = it.next();
        QString version = QFileInfo(path).fileName().section('.', 0, 0);

        if (!applied.contains(version)) {
            qDebug() << "Applying migration:" << version;
            if (!applyMigration(path)) {
                success = false;
                break;
            }
        }
    }

    return success;
}

// ----------------------------
// Apply a single migration file
// ----------------------------
bool DatabaseManager::applyMigration(const QString& sqlResourcePath)
{
    if (!initSchemaFromResource(sqlResourcePath)) {
        qCritical() << "Migration failed:" << sqlResourcePath;
        return false;
    }

    QString version = QFileInfo(sqlResourcePath).fileName().section('.', 0, 0);
    QSqlQuery query(m_db);
    if (!query.exec(QString("INSERT INTO schema_migrations (version, applied_at) "
                            "VALUES ('%1', CURRENT_TIMESTAMP)").arg(version))) {
        qCritical() << "Failed to record migration version:" << version
                    << query.lastError().text();
        return false;
    }

    qDebug() << "Migration applied:" << version;
    return true;
}

// ----------------------------
// Load and execute SQL from resource
// ----------------------------
bool DatabaseManager::initSchemaFromResource(const QString& sqlResourcePath)
{
    QFile file(sqlResourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open SQL resource:" << sqlResourcePath;
        return false;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    QStringList statements = sql.split(';', Qt::SkipEmptyParts);
    QSqlQuery query(m_db);
    bool success = true;

    for (QString stmt : statements) {
        stmt = stmt.trimmed();
        if (stmt.isEmpty()) continue;
        if (!query.exec(stmt)) {
            qCritical() << "Failed SQL statement:\n" << stmt
                        << "\nError:" << query.lastError().text();
            success = false;
        }
    }

    return success;
}

// ----------------------------
// Ensure schema_migrations table exists
// ----------------------------
bool DatabaseManager::initMigrationSchema()
{
    QSqlQuery query(m_db);
    return query.exec(R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version TEXT PRIMARY KEY,
            applied_at DATETIME NOT NULL
        )
    )");
}

#include "databasemanager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager _instance;
    return _instance;
}

bool DatabaseManager::openDatabase(const QString& path) {
    QString dbPath = path;
    if (dbPath.isEmpty()) {
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataPath);
        dbPath = dataPath + "/restaurant.db";
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }
    return initSchema();
}

void DatabaseManager::closeDatabase() {
    QString connectionName = m_db.connectionName();
    if (m_db.isOpen()) m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

bool DatabaseManager::initSchema() {
    QSqlQuery query(m_db);
    bool success = true;

    success &= query.exec("CREATE TABLE IF NOT EXISTS orders (id TEXT PRIMARY KEY, table_number INTEGER, status INTEGER, created_at DATETIME)");
    success &= query.exec("CREATE TABLE IF NOT EXISTS order_items (id TEXT PRIMARY KEY, order_id TEXT, menu_item_id INTEGER, name TEXT, quantity INTEGER, price_cents INTEGER, FOREIGN KEY(order_id) REFERENCES orders(id))");
    success &= query.exec("CREATE TABLE IF NOT EXISTS categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE)");
    success &= query.exec("CREATE TABLE IF NOT EXISTS menu_items (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, category TEXT, base_price_cents INTEGER, icon_source TEXT, FOREIGN KEY(category) REFERENCES categories(name))");
    success &= query.exec("CREATE TABLE IF NOT EXISTS inventory (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, quantity INTEGER DEFAULT 0, unit TEXT)");
    success &= query.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, salt TEXT NOT NULL, role TEXT)");

    success &= query.exec("CREATE TABLE IF NOT EXISTS inventory_history (history_id INTEGER PRIMARY KEY AUTOINCREMENT, item_id INTEGER, item_name TEXT, action TEXT, old_quantity INTEGER, new_quantity INTEGER, change_details TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY(item_id) REFERENCES inventory(id) ON DELETE SET NULL)");

    success &= query.exec("CREATE TRIGGER IF NOT EXISTS log_inventory_insert AFTER INSERT ON inventory BEGIN INSERT INTO inventory_history (item_id, item_name, action, new_quantity, change_details) VALUES (NEW.id, NEW.name, 'ADD', NEW.quantity, 'Initial stock added'); END;");
    success &= query.exec("CREATE TRIGGER IF NOT EXISTS log_inventory_update AFTER UPDATE ON inventory WHEN OLD.quantity <> NEW.quantity BEGIN INSERT INTO inventory_history (item_id, item_name, action, old_quantity, new_quantity, change_details) VALUES (NEW.id, NEW.name, 'UPDATE', OLD.quantity, NEW.quantity, 'Stock adjusted from ' || OLD.quantity || ' to ' || NEW.quantity); END;");

    return success;
}

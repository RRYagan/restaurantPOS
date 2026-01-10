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
        qDebug() << "Database initialized at:" << dbPath;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }
    initSchema();
    seeder.seed();

    return true;
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

    // --- MODIFIER DEFINITIONS TABLE ---
    success &= query.exec("CREATE TABLE IF NOT EXISTS menu_item_modifiers ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "menu_item_id INTEGER, "
                          "name TEXT NOT NULL, "
                          "extra_price_cents INTEGER DEFAULT 0, "
                          "FOREIGN KEY(menu_item_id) REFERENCES menu_items(id) ON DELETE CASCADE);");

    // --- ORDER ITEM MODIFIERS TABLE ---
    success &= query.exec("CREATE TABLE IF NOT EXISTS order_item_modifiers ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "order_item_id TEXT, " // Matches UUID in order_items
                          "modifier_name TEXT, "
                          "extra_price_cents INTEGER, "
                          "FOREIGN KEY(order_item_id) REFERENCES order_items(id) ON DELETE CASCADE);");

    // --- MODIFIER LOGGING TRIGGER ---
    // Records whenever a price change occurs for a modifier definition
    // success &= query.exec("CREATE TRIGGER IF NOT EXISTS log_modifier_price_change "
    //                       "AFTER UPDATE ON menu_item_modifiers "
    //                       "WHEN OLD.extra_price_cents <> NEW.extra_price_cents "
    //                       "BEGIN "
    //                       "INSERT INTO inventory_history (item_id, item_name, action, old_quantity, new_quantity, change_details) "
    //                       "VALUES (NEW.id, NEW.name, 'PRICE_UPDATE', 0, 0, "
    //                       "'Modifier price changed from ' || OLD.extra_price_cents || ' to ' || NEW.extra_price_cents); "
    //                       "END;");

    // payment
    success &= query.exec("CREATE TABLE IF NOT EXISTS payments ("
                          "id TEXT PRIMARY KEY, "           // M-Pesa Receipt or internal UUID
                          "order_id TEXT, "                 // Reference to orders table
                          "payment_type TEXT, "             // cash, mpesa, card, airtel
                          "amount_cents INTEGER, "
                          "status TEXT, "                   // Initiated, Completed, Failed
                          "user_tag TEXT, "                 // Tag attached to transaction
                          "external_reference TEXT, "       // CheckoutRequestID for M-Pesa
                          "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
                          "FOREIGN KEY(order_id) REFERENCES orders(id));");

    return success;
}

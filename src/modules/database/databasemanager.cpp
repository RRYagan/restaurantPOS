#include "databasemanager.h"
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent){}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager _instance;
    return _instance;
}

bool DatabaseManager::openDatabase() {
    // 1. Define where the file should live
    // This creates a path like: /home/user/.local/share/RestaurantPOS/pos_data.db
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath); // Ensure the folder exists

    QString dbPath = dataPath + "/restaurant.db";
    qDebug() << "Database Path:" << dbPath;

    // 2. Setup the connection
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    // 3. Open the file (This creates the .db file if it's missing)
    if (!m_db.open()) {
        qCritical() << "Error: Connection with database failed" << m_db.lastError().text();
        return false;
    }

    if (m_db.isOpen()) {
        if (initSchema()) {
            seedDatabase(); // Seed right after schema creation
            return true;
        }
    }
    return false;
}

bool DatabaseManager::initSchema() {

    QSqlQuery query;



    // 1. Menu Items Table

    query.exec("CREATE TABLE IF NOT EXISTS menu_items ("

               "id INTEGER PRIMARY KEY AUTOINCREMENT,"

               " name TEXT NOT NULL,"

               "category TEXT,"

               "base_price_cents INTEGER,"

               "icon_source TEXT)");



    // 2. Orders Table

    // Orders Table
    query.exec("CREATE TABLE IF NOT EXISTS orders ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "table_number INTEGER,"
               "status TEXT DEFAULT 'OPEN',"
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)");

    // Order Items Table
    query.exec("CREATE TABLE IF NOT EXISTS order_items ("
               "id TEXT PRIMARY KEY," // Requires UUID string from C++
               "order_id INTEGER,"
               "menu_item_id INTEGER,"
               "name TEXT,"
               "quantity INTEGER,"
               "price_cents INTEGER,"
               "FOREIGN KEY(order_id) REFERENCES orders(id))");



    return true;

}
void DatabaseManager::seedDatabase() {
    QSqlQuery checkQuery("SELECT COUNT(*) FROM menu_items");
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        return; // Database already has data, don't seed again
    }

    m_db.transaction(); // Use a transaction for much faster bulk inserts

    QSqlQuery q;
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) "
              "VALUES (?, ?, ?, ?)");

    // Helper lambda to make adding items cleaner
    auto addItem = [&](QString name, QString cat, int price, QString icon) {
        q.addBindValue(name);
        q.addBindValue(cat);
        q.addBindValue(price);
        q.addBindValue(icon);
        q.exec();
    };

    // --- SEED DATA ---

    // Category: Burgers
    addItem("Classic Cheeseburger", "Burgers", 1250, "qrc:/assets/icons/burger.svg");
    addItem("Bacon BBQ Burger", "Burgers", 1450, "qrc:/assets/icons/burger_bacon.svg");
    addItem("Veggie Deluxe", "Burgers", 1100, "qrc:/assets/icons/veggie.svg");

    // Category: Drinks
    addItem("Craft Beer IPA", "Drinks", 700, "qrc:/assets/icons/beer.svg");
    addItem("Fresh Lemonade", "Drinks", 450, "qrc:/assets/icons/lemonade.svg");
    addItem("Espresso", "Drinks", 350, "qrc:/assets/icons/coffee.svg");

    // Category: Sides
    addItem("Truffle Fries", "Sides", 650, "qrc:/assets/icons/fries.svg");
    addItem("Caesar Salad", "Sides", 800, "qrc:/assets/icons/salad.svg");
    addItem("Onion Rings", "Sides", 550, "qrc:/assets/icons/rings.svg");

    // Category: Desserts
    addItem("New York Cheesecake", "Desserts", 900, "qrc:/assets/icons/cake.svg");
    addItem("Chocolate Brownie", "Desserts", 750, "qrc:/assets/icons/brownie.svg");

    m_db.commit();
    qDebug() << "Database seeded successfully with initial menu items.";
}

bool DatabaseManager::saveOrder(const Order& order) {
    m_db.transaction(); // Atomic save: Order + all Items

    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO orders (id, table_number, created_at, status) "
              "VALUES (:id, :table, :date, :status)");
    q.bindValue(":id", order.orderId);
    q.bindValue(":table", order.tableNumber);
    q.bindValue(":date", order.createdAt.toString(Qt::ISODate));
    q.bindValue(":status", static_cast<int>(order.status));

    if (!q.exec()) { m_db.rollback(); return false; }

    // Clear old items and re-insert (simplest way to sync list)
    QSqlQuery d;
    d.prepare("DELETE FROM order_items WHERE order_id = ?");
    d.addBindValue(order.orderId);
    d.exec();

    for (const auto& item : order.items) {
        QSqlQuery i;
        i.prepare("INSERT INTO order_items (unique_id, order_id, menu_item_id, name, quantity, price_cents) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
        i.addBindValue(item.uniqueId.toString());
        i.addBindValue(order.orderId);
        i.addBindValue(item.menuItemId);
        i.addBindValue(item.name);
        i.addBindValue(item.quantity);
        i.addBindValue(static_cast<qlonglong>(item.priceAtTimeOfSale.cents));
        i.exec();
    }

    return m_db.commit();
}

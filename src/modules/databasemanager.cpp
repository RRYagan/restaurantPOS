#include "databasemanager.h"
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QSqlRecord>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager _instance;
    return _instance;
}

// databasemanager.cpp
bool DatabaseManager::openDatabase(const QString& path) {
    QString dbPath;

    if (path.isEmpty()) {
        // Default production path
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataPath);
        dbPath = dataPath + "/restaurant.db";
    } else {
        // Custom path (e.g., ":memory:" for tests)
        dbPath = path;
    }

    // Use a unique connection name if needed, or stick to default
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }

    if (initSchema()) {
        seedDatabase();
        return true;
    }
    return false;
}
void DatabaseManager::closeDatabase() {
    // We must capture the connection name before the database object is modified
    QString connectionName = m_db.connectionName();

    // 1. Close the actual connection if it's open
    if (m_db.isOpen()) {
        m_db.close();
    }

    // 2. Clear the QSqlDatabase object to release its handle on the connection
    m_db = QSqlDatabase();

    // 3. Remove the connection from the global registry using its name
    // This allows openDatabase() to be called again without "duplicate connection" warnings.
    QSqlDatabase::removeDatabase(connectionName);
}
bool DatabaseManager::initSchema() {
    QSqlQuery query;
    bool success = true;

    // Menu Items
    success &= query.exec("CREATE TABLE IF NOT EXISTS menu_items ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL, "
                          "category TEXT, "
                          "base_price_cents INTEGER, "
                          "icon_source TEXT)");

    // Orders
    success &= query.exec("CREATE TABLE IF NOT EXISTS orders ("
                          "id TEXT PRIMARY KEY, "
                          "table_number INTEGER, "
                          "status INTEGER, " // Changed to INTEGER to match enum cast
                          "created_at DATETIME)");

    // Order Items (Fixed column name to 'id' to match saveOrder logic)
    success &= query.exec("CREATE TABLE IF NOT EXISTS order_items ("
                          "id TEXT PRIMARY KEY, "
                          "order_id TEXT, "
                          "menu_item_id INTEGER, "
                          "name TEXT, "
                          "quantity INTEGER, "
                          "price_cents INTEGER, "
                          "FOREIGN KEY(order_id) REFERENCES orders(id))");

    return success;
}

// In databasemanager.cpp
bool DatabaseManager::saveOrder(Order &order) {

    if (!m_db.transaction()) return false;

    try {
        QSqlQuery q;
        // 2. INSERT OR REPLACE handles updating the main 'orders' entry
        q.prepare("INSERT OR REPLACE INTO orders (id, table_number, status, created_at) "
                  "VALUES (?, ?, ?, ?)");
        q.addBindValue(order.orderId);
        q.addBindValue(order.tableNumber);
        q.addBindValue(static_cast<int>(order.status));
        q.addBindValue(order.createdAt.toString(Qt::ISODate));

        if (!q.exec()) throw std::runtime_error("Order table update failed");

        // 3. Clear existing items for this order (Standard for Update logic)
        QSqlQuery del;
        del.prepare("DELETE FROM order_items WHERE order_id = ?");
        del.addBindValue(order.orderId);
        del.exec();

        // 4. Insert the current list of items
        for (const auto &item : order.items) {
            QSqlQuery iq;
            iq.prepare("INSERT INTO order_items (id, order_id, menu_item_id, name, quantity, price_cents) "
                       "VALUES (?, ?, ?, ?, ?, ?)");
            // Every order_item record gets a unique UUID
            iq.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
            iq.addBindValue(order.orderId); // Links to the main order
            iq.addBindValue(item.menuItemId);
            iq.addBindValue(item.name);
            iq.addBindValue(item.quantity);
            iq.addBindValue(static_cast<qlonglong>(item.price.cents));

            if (!iq.exec()) throw std::runtime_error("Item insertion failed");
        }

        return m_db.commit();

    } catch (const std::exception& e) {
        qDebug() << "Database Error:" << e.what();
        m_db.rollback();
        return false;
    }
}
Order DatabaseManager::loadOrder(const QString& orderId) {
    Order order;
    QSqlQuery q;

    q.prepare("SELECT table_number, status, created_at FROM orders WHERE id = ?");
    q.addBindValue(orderId);

    if (q.exec() && q.next()) {
        order.orderId = orderId;
        order.tableNumber = q.value(0).toInt();
        order.status = static_cast<OrderStatus>(q.value(1).toInt());
        order.createdAt = QDateTime::fromString(q.value(2).toString(), Qt::ISODate);

        QSqlQuery itemQuery;
        itemQuery.prepare("SELECT id, menu_item_id, name, quantity, price_cents "
                          "FROM order_items WHERE order_id = ?");
        itemQuery.addBindValue(orderId);

        while (itemQuery.next()) {
            OrderItem item;
            item.id = itemQuery.value(0).toString();
            item.menuItemId = itemQuery.value(1).toInt();
            item.name = itemQuery.value(2).toString();
            item.quantity = itemQuery.value(3).toInt();
            item.price.cents = itemQuery.value(4).toLongLong();
            order.items.append(item);
        }
    }
    return order;
}



void DatabaseManager::seedDatabase() {
    QSqlQuery check("SELECT COUNT(*) FROM menu_items");
    if (check.next() && check.value(0).toInt() > 0) return;

    m_db.transaction();
    QSqlQuery q;
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");

    auto addItem = [&](QString n, QString c, int p, QString i) {
        q.addBindValue(n);
        q.addBindValue(c);
        q.addBindValue(p);
        q.addBindValue(i);
        q.exec();
    };

    // --- BURGERS ---
    addItem("Classic Cheeseburger", "Burgers", 1250, "qrc:/assets/icons/burger.svg");
    addItem("Bacon Blue Burger", "Burgers", 1450, "qrc:/assets/icons/burger.svg");
    addItem("Veggies Delight Burger", "Burgers", 1100, "qrc:/assets/icons/burger.svg");

    // --- MAINS ---
    addItem("Grilled Ribeye Steak", "Mains", 2800, "qrc:/assets/icons/steak.svg");
    addItem("Pan-Seared Salmon", "Mains", 2400, "qrc:/assets/icons/fish.svg");
    addItem("Wild Mushroom Risotto", "Mains", 1850, "qrc:/assets/icons/pasta.svg");

    // --- APPETIZERS / BAR SNACKS ---
    addItem("Buffalo Wings (8pcs)", "Appetizers", 950, "qrc:/assets/icons/wings.svg");
    addItem("Truffle Fries", "Appetizers", 650, "qrc:/assets/icons/fries.svg");
    addItem("Calamari Rings", "Appetizers", 1100, "qrc:/assets/icons/seafood.svg");

    // --- DRINKS (NON-ALCOHOLIC) ---
    addItem("Fresh Lemonade", "Drinks", 450, "qrc:/assets/icons/lemonade.svg");
    addItem("Sparkling Water", "Drinks", 300, "qrc:/assets/icons/water.svg");
    addItem("Iced Peach Tea", "Drinks", 500, "qrc:/assets/icons/tea.svg");

    // --- BAR (ALCOHOLIC) ---
    addItem("Craft IPA Beer", "Bar", 750, "qrc:/assets/icons/beer.svg");
    addItem("Old Fashioned Cocktail", "Bar", 1200, "qrc:/assets/icons/cocktail.svg");
    addItem("Chardonnay (Glass)", "Bar", 900, "qrc:/assets/icons/wine.svg");
    addItem("Cabernet Sauvignon (Bottle)", "Bar", 4500, "qrc:/assets/icons/wine_bottle.svg");

    // --- DESSERTS ---
    addItem("New York Cheesecake", "Desserts", 850, "qrc:/assets/icons/cake.svg");
    addItem("Chocolate Lava Cake", "Desserts", 950, "qrc:/assets/icons/cake.svg");

    m_db.commit();
}

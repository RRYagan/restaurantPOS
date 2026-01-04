#include "databasemanager.h"
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QUuid>

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

    // Categories Table
    success &= query.exec("CREATE TABLE IF NOT EXISTS categories ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL UNIQUE)");

    // Menu Items Table
    success &= query.exec("CREATE TABLE IF NOT EXISTS menu_items ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL, "
                          "category TEXT, "
                          "base_price_cents INTEGER, "
                          "icon_source TEXT, "
                          "FOREIGN KEY(category) REFERENCES categories(name))");

    // Inventory Table
    success &= query.exec("CREATE TABLE IF NOT EXISTS inventory ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "name TEXT NOT NULL, "
                          "quantity INTEGER DEFAULT 0, "
                          "unit TEXT)");

    // Users Table (Basic auth)
    success &= query.exec("CREATE TABLE IF NOT EXISTS users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "username TEXT NOT NULL UNIQUE, "
                          "password_hash TEXT NOT NULL, "
                          "salt TEXT NOT NULL, "
                          "role TEXT)");

    return success;
}
void DatabaseManager::seedDatabase() {
    // 1. Check if we already have data
    QSqlQuery check("SELECT COUNT(*) FROM categories");
    if (check.next() && check.value(0).toInt() > 0) return;

    m_db.transaction();

    // 2. Seed Categories
    QStringList categories = {"Burgers", "Mains", "Appetizers", "Drinks", "Bar", "Desserts"};
    QSqlQuery catQuery;
    catQuery.prepare("INSERT INTO categories (name) VALUES (?)");
    for (const QString &cat : categories) {
        catQuery.addBindValue(cat);
        catQuery.exec();
    }

    // 3. Seed Menu Items using the seeded categories
    QSqlQuery q;
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");

    auto addItem = [&](QString n, QString c, int p, QString i) {
        q.addBindValue(n);
        q.addBindValue(c);
        q.addBindValue(p);
        q.addBindValue(i);
        q.exec();
    };

    // BURGERS
    addItem("Classic Cheeseburger", "Burgers", 1250, "qrc:/assets/icons/burger.svg");
    addItem("Bacon Blue Burger", "Burgers", 1450, "qrc:/assets/icons/burger.svg");

    // DRINKS
    addItem("Fresh Lemonade", "Drinks", 450, "qrc:/assets/icons/lemonade.svg");
    addItem("Sparkling Water", "Drinks", 300, "qrc:/assets/icons/water.svg");

    // BAR
    addItem("Craft IPA Beer", "Bar", 750, "qrc:/assets/icons/beer.svg");
    addItem("Old Fashioned", "Bar", 1200, "qrc:/assets/icons/cocktail.svg");

    // Seed Inventory
    QSqlQuery inv;
    inv.prepare("INSERT INTO inventory (name, quantity, unit) VALUES (?, ?, ?)");
    auto addInv = [&](QString n, int q, QString u) {
        inv.addBindValue(n); inv.addBindValue(q); inv.addBindValue(u);
        inv.exec();
    };
    addInv("Beef Patties", 100, "pcs");
    addInv("Burger Buns", 120, "pcs");
    addInv("Milk", 50, "liters");

    // Seed Users
    // QSqlQuery userQ;
    // userQ.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");

    addUser("admin", "admin123", "manager");
    addUser("waiter1", "password", "staff");

    m_db.commit();
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

//menu

// --- ADD ITEM ---
bool DatabaseManager::addMenuItem(const QString &name, const QString &category, int priceCents, const QString &icon) {
    QSqlQuery q;
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(priceCents);
    q.addBindValue(icon);
    return q.exec();
}

// --- EDIT / UPDATE ITEM ---
bool DatabaseManager::updateMenuItem(int id, const QString &name, const QString &category, int priceCents, const QString &icon) {
    QSqlQuery q;
    q.prepare("UPDATE menu_items SET name = ?, category = ?, base_price_cents = ?, icon_source = ? WHERE id = ?");
    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(priceCents);
    q.addBindValue(icon);
    q.addBindValue(id);
    return q.exec();
}

// --- DELETE ITEM ---
bool DatabaseManager::deleteMenuItem(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM menu_items WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

QVector<MenuItem> DatabaseManager::fetchMenuItems(const QString &categoryFilter) {
    QVector<MenuItem> items;
    QSqlQuery query;

    if (categoryFilter == "All") {
        query.prepare("SELECT id, name, category, base_price_cents, icon_source FROM menu_items");
    } else {
        query.prepare("SELECT id, name, category, base_price_cents, icon_source FROM menu_items WHERE category = ?");
        query.addBindValue(categoryFilter);
    }

    if (query.exec()) {
        while (query.next()) {
            MenuItem item;
            item.id = query.value(0).toInt();
            item.name = query.value(1).toString();
            item.category = query.value(2).toString();
            item.basePrice.cents = query.value(3).toLongLong();
            item.iconSource = query.value(4).toString();
            items.append(item);
        }
    }
    return items;
}


// Categories
QStringList DatabaseManager::fetchCategories() {
    QStringList list;
    list << "All";
    QSqlQuery query("SELECT name FROM categories ORDER BY name ASC");
    while (query.next()) {
        list << query.value(0).toString();
    }
    return list;
}
bool DatabaseManager::deleteCategory(const QString& name) {
    if (!m_db.transaction()) return false;

    QSqlQuery q;
    // 1. Delete all items associated with this category
    q.prepare("DELETE FROM menu_items WHERE category = ?");
    q.addBindValue(name);
    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    // 2. Delete the category from the categories table
    q.prepare("DELETE FROM categories WHERE name = ?");
    q.addBindValue(name);
    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

bool DatabaseManager::addCategory(const QString& name) {
    QSqlQuery q;
    q.prepare("INSERT INTO categories (name) VALUES (?)");
    q.addBindValue(name);

    if (!q.exec()) {
        qWarning() << "Error adding category:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateCategory(const QString& oldName, const QString& newName) {
    if (!m_db.transaction()) return false;

    QSqlQuery q;
    // 1. Update the category name in the categories table
    q.prepare("UPDATE categories SET name = ? WHERE name = ?");
    q.addBindValue(newName);
    q.addBindValue(oldName);

    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    // 2. Update all menu items that were using the old category name
    q.prepare("UPDATE menu_items SET category = ? WHERE category = ?");
    q.addBindValue(newName);
    q.addBindValue(oldName);

    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// inventory
bool DatabaseManager::addInventoryItem(const QString &name, int quantity, const QString &unit) {
    QSqlQuery q;
    q.prepare("INSERT INTO inventory (name, quantity, unit) VALUES (?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(quantity);
    q.addBindValue(unit);
    return q.exec();
}

bool DatabaseManager::updateInventoryItem(int id, const QString &name, int quantity, const QString &unit) {
    QSqlQuery q;
    q.prepare("UPDATE inventory SET name = ?, quantity = ?, unit = ? WHERE id = ?");
    q.addBindValue(name);
    q.addBindValue(quantity);
    q.addBindValue(unit);
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::deleteInventoryItem(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM inventory WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}


// usser
bool DatabaseManager::addUser(const QString &username, const QString &password, const QString &role) {
    QString salt = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8); // Generate 8-char salt
    QString hash = hashPassword(password, salt);

    QSqlQuery q;
    q.prepare("INSERT INTO users (username, password_hash, salt, role) VALUES (?, ?, ?, ?)");
    q.addBindValue(username);
    q.addBindValue(hash);
    q.addBindValue(salt);
    q.addBindValue(role);
    return q.exec();
}

bool DatabaseManager::verifyUser(const QString &username, const QString &password) {
    QSqlQuery q;
    // We select id and role as well to populate the session
    q.prepare("SELECT id, password_hash, salt, role FROM users WHERE username = ?");
    q.addBindValue(username);

    if (q.exec() && q.next()) {
        int id = q.value(0).toInt();
        QString storedHash = q.value(1).toString();
        QString salt = q.value(2).toString();
        QString role = q.value(3).toString();

        if (hashPassword(password, salt) == storedHash) {
            // Populate session data
            m_session.userId = id;
            m_session.username = username;
            m_session.role = role;
            m_session.isValid = true;
            return true;
        }
    }

    m_session = UserSession(); // Reset session on failure
    return false;
}

// Helper to hash password with a salt
QString DatabaseManager::hashPassword(const QString& password, const QString& salt) {
    QByteArray saltedPassword = (password + salt).toUtf8();
    QByteArray hash = QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}
bool DatabaseManager::deleteUser(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM users WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

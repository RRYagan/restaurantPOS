#include "databaseseeder.h"
#include "databasemanager.h"
#include "usermodel.h"
#include <QSqlQuery>
void DatabaseSeeder::seed() {
    QSqlDatabase db = DatabaseManager::instance().database();
    QSqlQuery check("SELECT COUNT(*) FROM categories", db);
    if (check.next() && check.value(0).toInt() > 0) return;

    db.transaction();
    QSqlQuery query(db);


    // --- 2. CATEGORIES ---
    QStringList categories = {"Appetizers", "Burgers", "Steaks", "Pasta", "Pizza", "Salads", "Desserts", "Cocktails", "Wine", "Coffee"};
    query.prepare("INSERT INTO categories (name) VALUES (?)");
    for (const auto& cat : categories) {
        query.addBindValue(cat);
        query.exec();
    }

    // --- 3. HELPER FUNCTIONS ---
    auto addItem = [&](QString n, QString c, int p, QString i) {
        QSqlQuery q(db);
        q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");
        q.addBindValue(n); q.addBindValue(c); q.addBindValue(p); q.addBindValue(i);
        q.exec();
        return q.lastInsertId().toInt();
    };

    auto addMod = [&](int mid, QString n, int p) {
        QSqlQuery m(db);
        m.prepare("INSERT INTO menu_item_modifiers (menu_item_id, name, extra_price_cents) VALUES (?, ?, ?)");
        m.addBindValue(mid); m.addBindValue(n); m.addBindValue(p);
        m.exec();
    };

    // --- 4. DATA POPULATION ---

    // BURGERS
    int b1 = addItem("Truffle Mushroom Burger", "Burgers", 1650, "qrc:/assets/icons/burger.svg");
    for(auto& m : QMap<QString, int>{{"Extra Patty", 450}, {"Bacon", 200}, {"Swiss Cheese", 100}}.toStdMap())
        addMod(b1, m.first, m.second);

    int b2 = addItem("Spicy Zinger Burger", "Burgers", 1300, "qrc:/assets/icons/burger.svg");
    addMod(b2, "Double Spice", 0);
    addMod(b2, "Cheddar Slice", 80);

    // STEAKS & MAINS
    int s1 = addItem("Prime Ribeye 400g", "Steaks", 4500, "qrc:/assets/icons/steak.svg");
    for(auto& m : QMap<QString, int>{{"Rare", 0}, {"Medium Rare", 0}, {"Medium", 0}, {"Well Done", 0}}.toStdMap())
        addMod(s1, m.first, m.second);
    addMod(s1, "Garlic Butter Topping", 150);

    int s2 = addItem("Grilled Salmon Fillet", "Steaks", 2800, "qrc:/assets/icons/fish.svg");
    addMod(s2, "Lemon Butter Sauce", 200);

    // COFFEE & DRINKS
    int c1 = addItem("Cappuccino", "Coffee", 450, "qrc:/assets/icons/coffee.svg");
    addMod(c1, "Oat Milk", 70);
    addMod(c1, "Almond Milk", 70);
    addMod(c1, "Extra Shot", 100);
    addMod(c1, "Decaf", 0);

    int d1 = addItem("Old Fashioned", "Cocktails", 1200, "qrc:/assets/icons/cocktail.svg");
    addMod(d1, "Premium Bourbon Upgrade", 500);

    // --- 5. LARGE INVENTORY SEED ---
    QList<QPair<QString, QString>> stockItems = {
        {"Beef Patties", "pcs"}, {"Ribeye Steak", "kg"}, {"Salmon", "kg"},
        {"Brioche Buns", "pcs"}, {"Potatoes", "kg"}, {"Coffee Beans", "kg"},
        {"Milk Full Cream", "L"}, {"Bourbon", "L"}, {"Cheddar Cheese", "kg"}
    };
    query.prepare("INSERT INTO inventory (name, quantity, unit) VALUES (?, ?, ?)");
    for (const auto& item : stockItems) {
        query.addBindValue(item.first);
        query.addBindValue(100); // Default stock
        query.addBindValue(item.second);
        query.exec();
    }

    // --- 6. USERS ---
    UserModel userModel;
    userModel.addUser("admin", "admin123", "manager");
    userModel.addUser("staff1", "1234", "waiter");
    userModel.addUser("chef1", "1234", "kitchen");

    // payment
    query.prepare("INSERT INTO payments (id, order_id, payment_type, amount_cents, status, user_tag) "
                  "VALUES (?, ?, ?, ?, ?, ?)");

    // Dummy M-Pesa Transaction
    query.addBindValue("QNA12RT45X");
    query.addBindValue("ORD-1001");
    query.addBindValue("mpesa");
    query.addBindValue(450000); // 4500.00
    query.addBindValue("Completed");
    query.addBindValue("admin");
    query.exec();

    // Dummy Cash Transaction
    query.addBindValue("CASH-9982");
    query.addBindValue("ORD-1002");
    query.addBindValue("cash");
    query.addBindValue(120000); // 1200.00
    query.addBindValue("Completed");
    query.addBindValue("staff1");
    query.exec();

    db.commit();
}

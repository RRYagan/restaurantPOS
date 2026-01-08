#include "databaseseeder.h"
#include "databasemanager.h"
#include "usermodel.h"
#include <QSqlQuery>

void DatabaseSeeder::seed() {
    QSqlDatabase db = DatabaseManager::instance().database();
    QSqlQuery check("SELECT COUNT(*) FROM categories", db);
    if (check.next() && check.value(0).toInt() > 0) return;

    db.transaction();

    QSqlQuery catQuery(db);
    catQuery.prepare("INSERT INTO categories (name) VALUES (?)");
    for (const QString &cat : {"Burgers", "Mains", "Appetizers", "Drinks", "Bar", "Desserts"}) {
        catQuery.addBindValue(cat); catQuery.exec();
    }

    QSqlQuery q(db);
    q.prepare("INSERT INTO menu_items (name, category, base_price_cents, icon_source) VALUES (?, ?, ?, ?)");
    auto addItem = [&](QString n, QString c, int p, QString i) {
        q.addBindValue(n); q.addBindValue(c); q.addBindValue(p); q.addBindValue(i);
        q.exec();
    };
    addItem("Classic Cheeseburger", "Burgers", 1250, "qrc:/assets/icons/burger.svg");
    addItem("Fresh Lemonade", "Drinks", 450, "qrc:/assets/icons/lemonade.svg");

    QSqlQuery inv(db);
    inv.prepare("INSERT INTO inventory (name, quantity, unit) VALUES (?, ?, ?)");
    auto addInv = [&](QString n, int q, QString u) {
        inv.addBindValue(n); inv.addBindValue(q); inv.addBindValue(u);
        inv.exec();
    };
    addInv("Beef Patties", 100, "pcs");

    UserModel userModel;
    userModel.addUser("admin", "admin123", "manager");
    userModel.addUser("waiter1", "password", "staff");

    db.commit();
}

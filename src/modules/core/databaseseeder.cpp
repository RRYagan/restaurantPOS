#include "databaseseeder.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

DatabaseSeeder::DatabaseSeeder(QSqlDatabase db)
    : m_db(db)
{
}


bool DatabaseSeeder::isTableEmpty(const QString& tableName)
{
    QSqlQuery q(m_db);
    q.prepare(QString("SELECT 1 FROM %1 LIMIT 1").arg(tableName));
    return !q.exec() || !q.next();
}

// void DatabaseSeeder::seedIfNeeded()
// {
//     // Pick a canonical table that must exist
//     if (!isTableEmpty("menu_item_products")) {
//         qDebug() << "Database already seeded. Skipping.";
//         return;
//     }

//     qDebug() << "Seeding database (tables empty)";
//     forceSeed();
// }

bool DatabaseSeeder::isSeeded() const
{
    QSqlQuery q(m_db);
    q.exec("SELECT value FROM schema_info WHERE key='seeded'");
    return q.next() && q.value(0).toString() == "true";
}

bool DatabaseSeeder::markSeeded() const
{
    QSqlQuery q(m_db);
    return q.exec("UPDATE schema_info SET value='true' WHERE key='seeded'");
}

void DatabaseSeeder::seedIfNeeded()
{
    // Pick a canonical table that must exist
    if (!isTableEmpty("menu_item_products")) {
        qDebug() << "Database already seeded. Skipping.";
        return ;
    }

    qDebug() << "Seeding database (tables empty)";
    forceSeed();
}

void DatabaseSeeder::forceSeed()
{
    if (!m_db.isOpen()) {
        qCritical() << "Seeder: database not open";
        return;
    }

    if (!m_db.transaction()) {
        qCritical() << "Seeder: failed to start transaction"
                    << m_db.lastError().text();
        return;
    }

    QSqlQuery q(m_db);
    bool ok = true;

    auto exec = [&](const QString& sql) {
        if (!q.exec(sql)) {
            qCritical() << "Seed failed:" << q.lastError().text();
            qCritical() << "SQL:" << sql;
            return false;
        }
        return true;
    };

    ok &= exec(
        "INSERT OR IGNORE INTO products "
        "(id, item_nm, base_price_cents) VALUES "
        "('p1', 'Soda', 150),"
        "('p2', 'Bread', 100)"
        );

    // ok &= exec(
    //     "INSERT OR IGNORE INTO users "
    //     "(id, username, password_hash, role) VALUES "
    //     "('u1', 'admin', 'hash', 'ADMIN')"
    //     );

    // --- Menu Categories ---
    exec(R"(INSERT OR IGNORE INTO menu_categories (id, name) VALUES
        ('c1', 'Drinks'),
        ('c2', 'Food')
)");

    // --- Menu Items ---
    exec(R"(INSERT OR IGNORE INTO menu_items
        (id, name, description, category_id, base_price_cents)
        VALUES
        ('m1', 'Soda', 'Cold drink', 'c1', 200),
        ('m2', 'Burger Combo', 'Burger + Soda', 'c2', 800)
)");

    // --- Menu Item Recipes (menu_item_products) ---
    exec(R"(INSERT OR IGNORE INTO menu_item_products
        (menu_item_id, product_id, quantity)
        VALUES
        ('m1', 'p_water', 0.5),
        ('m1', 'p_sugar', 0.05),
        ('m1', 'p_soda', 1),
        ('m2', 'p_bread', 1),
        ('m2', 'p_soda', 1),
        ('m2', 'p_burger', 1)
)");

    if (!ok) {
        qCritical() << "Seeder failed, rolling back";
        m_db.rollback();
        return;
    }

    if (!m_db.commit()) {
        qCritical() << "Seeder commit failed:"
                    << m_db.lastError().text();
        m_db.rollback();
    } else {
        qDebug() << "Database seeded successfully";
    }
}


bool DatabaseSeeder::seedUsers() const
{
    QSqlQuery q(m_db);

    return q.exec(R"(
        INSERT OR IGNORE INTO users (id, username, password_hash, full_name, role)
        VALUES
        ('admin-uuid', 'admin', 'hashed_password', 'Admin User', 'ADMIN'),
        ('waiter-uuid', 'waiter', 'hashed_password', 'Waiter User', 'WAITER')
    )");
}

bool DatabaseSeeder::seedProducts() const
{
    QSqlQuery q(m_db);

    // NOTE:
    // item_ty_cd: 1 = Raw, 2 = Finished, 3 = Service
    // tax_ty_cd: B = Standard VAT
    return q.exec(R"(
        INSERT OR IGNORE INTO products
        (id, item_nm, item_cd, item_cls_cd, item_ty_cd, tax_ty_cd,
         pkg_unit_cd, qty_unit_cd, base_price_cents, is_available)
        VALUES
        -- Raw materials
        ('p_water',  'Water',  'WTR001', '1000000001', '1', 'B', 'BOT', 'LTR', 50, 1),
        ('p_sugar',  'Sugar',  'SGR001', '1000000002', '1', 'B', 'BAG', 'KG', 120, 1),
        ('p_bread',  'Bread',  'BRD001', '1000000003', '1', 'B', 'PKT', 'NO', 80, 1),

        -- Finished goods
        ('p_soda',   'Soda',   'SOD001', '2000000001', '2', 'B', 'BOT', 'NO', 200, 1),
        ('p_burger', 'Burger', 'BRG001', '2000000002', '2', 'B', 'NO',  'NO', 600, 1),

        -- Services
        ('p_service','Service Charge', 'SRV001', '3000000001', '3', 'E', NULL, NULL, 100, 1)
    )");
}

bool DatabaseSeeder::seedMenu() const
{
    QSqlQuery q(m_db);

    return q.exec(R"(
        INSERT OR IGNORE INTO menu_categories (id, name)
        VALUES
        ('cat_drinks', 'Drinks'),
        ('cat_food', 'Food');

        INSERT OR IGNORE INTO menu_items
        (id, name, category_id, base_price_cents)
        VALUES
        ('menu_soda', 'Soda', 'cat_drinks', 200),
        ('menu_burger', 'Burger', 'cat_food', 600);

        INSERT OR IGNORE INTO menu_item_products
        (menu_item_id, product_id, quantity)
        VALUES
        ('menu_soda', 'p_soda', 1),
        ('menu_burger', 'p_bread', 1),
        ('menu_burger', 'p_patty', 1),
        ('menu_burger', 'p_cheese', 1);
    )");
}

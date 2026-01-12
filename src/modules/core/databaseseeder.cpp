#include "databaseseeder.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseSeeder::DatabaseSeeder(QSqlDatabase db)
    : m_db(db)
{}

void DatabaseSeeder::seedIfNeeded()
{
    if (isSeeded()) {
        qDebug() << "Seed already applied.";
        return;
    }
    forceSeed();
}

void DatabaseSeeder::forceSeed()
{
    qDebug() << "Running database seed...";

    if (!m_db.transaction()) {
        qCritical() << m_db.lastError().text();
        return;
    }

    bool ok =
        seedUsers() &&
        seedProducts() &&
        seedProductCompositions() &&
        seedOpeningStockMovements() &&
        markSeeded();

    if (ok) {
        m_db.commit();
        qDebug() << "Seeding complete.";
    } else {
        m_db.rollback();
        qCritical() << "Seeding failed.";
    }
}

bool DatabaseSeeder::seedUsers()
{
    QSqlQuery q(m_db);
    return q.exec(R"(
        INSERT OR IGNORE INTO users
        (staff_id_number, full_name, user_role, login_username, password_hash)
        VALUES
        ('ADM001','Admin User','Admin','admin','pbkdf2_hash'),
        ('CSH001','Main Cashier','Cashier','cashier','pbkdf2_hash')
    )");
}

bool DatabaseSeeder::seedProducts()
{
    QSqlQuery q(m_db);

    bool a = q.exec(R"(
        INSERT OR IGNORE INTO product
        (kra_unique_item_code, internal_product_name, product_category_code, measurement_unit_id)
        VALUES
        ('RAW-BEEF-01','Beef Mince','1',(SELECT id FROM measurement_units WHERE unit_code='KG'))
    )");

    bool b = q.exec(R"(
        INSERT OR IGNORE INTO product
        (kra_unique_item_code, internal_product_name, product_category_code, measurement_unit_id)
        VALUES
        ('RAW-BUN-01','Burger Bun','1',(SELECT id FROM measurement_units WHERE unit_code='EA'))
    )");

    bool c = q.exec(R"(
        INSERT OR IGNORE INTO product
        (kra_unique_item_code, internal_product_name, product_category_code,
         default_selling_price, measurement_unit_id, tax_classification_id)
        VALUES
        ('FIN-BRGR-01','Double Beef Burger','2',1200,
         (SELECT id FROM measurement_units WHERE unit_code='EA'),
         (SELECT id FROM tax_classifications WHERE tax_type_code='A'))
    )");

    return a && b && c;
}

bool DatabaseSeeder::seedProductCompositions()
{
    QSqlQuery q(m_db);
    return q.exec(R"(
        INSERT OR IGNORE INTO product_composition
        (main_product_item_id, ingredient_item_id, required_quantity)
        VALUES
        (
          (SELECT id FROM product WHERE kra_unique_item_code='FIN-BRGR-01'),
          (SELECT id FROM product WHERE kra_unique_item_code='RAW-BEEF-01'),
          0.30
        ),
        (
          (SELECT id FROM product WHERE kra_unique_item_code='FIN-BRGR-01'),
          (SELECT id FROM product WHERE kra_unique_item_code='RAW-BUN-01'),
          1.0
        )
    )");
}

bool DatabaseSeeder::seedOpeningStockMovements()
{
    QSqlQuery q(m_db);
    return q.exec(R"(
        INSERT INTO inventory_movement_log
        (product_id, movement_quantity, movement_category_id)
        VALUES
        (
          (SELECT id FROM product WHERE kra_unique_item_code='RAW-BEEF-01'),
          50,
          (SELECT id FROM stock_movement_categories WHERE movement_type_code='01')
        ),
        (
          (SELECT id FROM product WHERE kra_unique_item_code='RAW-BUN-01'),
          100,
          (SELECT id FROM stock_movement_categories WHERE movement_type_code='01')
        )
    )");
}

bool DatabaseSeeder::isSeeded() const
{
    QSqlQuery q(m_db);
    q.exec("SELECT value FROM schema_info WHERE key='seeded'");
    return q.next() && q.value(0).toString() == "true";
}

bool DatabaseSeeder::markSeeded() const
{
    QSqlQuery q(m_db);
    return q.exec("INSERT OR REPLACE INTO schema_info VALUES ('seeded','true')");
}

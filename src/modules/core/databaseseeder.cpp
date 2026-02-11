#include "databaseseeder.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseSeeder::DatabaseSeeder(QSqlDatabase db)
    : m_db(db)
{
    seedTransactions();
}

void DatabaseSeeder::seedIfNeeded()
{
    if (isSeeded()) {
        qDebug() << "Seed already applied.";
        return;
    }
    seedTransactions();
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
        // seedUsers() &&
        // seedProducts() &&
        // seedProductCompositions() &&
        // seedOpeningStockMovements() &&
        seedTransactions() &&
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
bool DatabaseSeeder::seedTransactions()
{
    QSqlQuery q(m_db);

    // 1. Create some Waiters/Staff if not already in users table
    // (Assuming staff_id_number is used in customer_order.waiter_id)

    // 2. Insert Orders, Sales, and Payments for the last 30 days
    // We use a loop in SQL or multiple inserts to create a distribution
    qDebug() << "Seeding transactional data for reports...";

    // Seed a mix of Success and Pending/Failed orders
    bool ordersOk = q.exec(R"(
        INSERT INTO customer_order
        (id, waiter_id, order_status, gross_amount, created_at)
        VALUES
        ('ORD-001', 'ADM001', 'Completed', 1200.0, '2026-02-01 09:30:00'),
        ('ORD-002', 'CSH001', 'Completed', 2500.0, '2026-02-01 13:15:00'),
        ('ORD-003', 'CSH001', 'Completed', 850.0,  '2026-02-02 14:00:00'),
        ('ORD-004', 'ADM001', 'Completed', 6000.0, '2026-02-02 19:45:00'),
        ('ORD-005', 'CSH001', 'Completed', 3200.0, '2026-02-03 12:00:00'),
        ('ORD-006', 'CSH001', 'Completed', 150.0,  '2026-02-03 12:30:00');
    )");

    bool salesOk = q.exec(R"(
        INSERT INTO sale
        (order_id, gross_amount, sale_transaction_date, kra_receipt_number, payment_method_id)
        VALUES
        ('ORD-001', 1200.0, '2026-02-01 09:35:00', 'KRA-SIG-001', 1),
        ('ORD-002', 2500.0, '2026-02-01 13:20:00', 'KRA-SIG-002', 1),
        ('ORD-003', 850.0,  '2026-02-02 14:05:00', 'MISSING',     2),
        ('ORD-004', 6000.0, '2026-02-02 19:50:00', 'KRA-SIG-004', 1),
        ('ORD-005', 3200.0, '2026-02-03 12:05:00', 'KRA-SIG-005', 2),
        ('ORD-006', 150.0,  '2026-02-03 12:35:00', 'MISSING',     1);
    )");

    bool paymentsOk = q.exec(R"(
        INSERT INTO payments
        (order_id, amount, status, external_reference)
        VALUES
        ('ORD-001', 1200.0, 'Success', 'MPESA-REF-01'),
        ('ORD-002', 2500.0, 'Success', 'CASH-01'),
        ('ORD-003', 850.0,  'Success', 'CASH-02'),
        ('ORD-004', 6000.0, 'Success', 'CARD-REF-99'),
        ('ORD-005', 3200.0, 'Success', 'MPESA-REF-02'),
        ('ORD-006', 150.0,  'Success', 'CASH-03');
    )");

    return ordersOk && salesOk && paymentsOk;
}

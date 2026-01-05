#include <QtTest>
#include <QSignalSpy>
#include "databasemanager.h"
#include "salesview.h"
#include "menuview.h"
#include "money.h"
#include "order.h"

class RestaurantPOSTest : public QObject {
    Q_OBJECT

private slots:

    void initTestCase() {
        // Use an in-memory database to ensure isolation and speed
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_connection");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Use the singleton instance to set up schema for the model to use
        QVERIFY(DatabaseManager::instance().openDatabase());
    }

    // Run once after all tests
    void cleanupTestCase() {
        DatabaseManager::instance().closeDatabase();
    }

    void testSingletonEnforcement() {
        // Ensure static instance is unique
        DatabaseManager& instance1 = DatabaseManager::instance();
        DatabaseManager& instance2 = DatabaseManager::instance();
        QCOMPARE(&instance1, &instance2);
    }

    // --- FINANCIAL LOGIC (money.h) ---
    void testMoneyPrecision() {
        Money m1{1000}; // 10.00
        Money m2{555};  // 5.55

        // Addition
        Money sum = m1 + m2;
        QCOMPARE(sum.cents, (int64_t)1555);
        QCOMPARE(sum.toString(), QString("15.55"));

        // Multiplication (Tax calculation)
        Money taxed = m1 * 0.0825;
        QCOMPARE(taxed.cents, (int64_t)82); // 0.825 rounded to 82 cents
    }

    // --- ORDER CALCULATIONS (order.h) ---
    void testOrderGrandTotal() {
        Order order;
        OrderItem item;
        item.price = {1000};
        item.quantity = 2; // 20.00
        order.items.append(item);

        // Subtotal (20.00) + Tax (8% = 1.60) = 21.60
        QCOMPARE(order.grandTotal().cents, (int64_t)2160);
    }

    // --- SALES MODEL STATE & SIGNALS (salesmodel.cpp) ---
    void testSalesViewSignalsAndState() {
        SalesView model;
        QSignalSpy spy(&model, &SalesView::totalChanged);

        // Test addItemToOrder (Uses DB fetch)
        model.addItemToOrder(1); // Classic Cheeseburger (12.50)
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(spy.count(), 1); // Verify totalChanged signal was emitted for UI

        // Test Quantity Update logic
        model.updateQuantity(0, 3);
        QCOMPARE(model.data(model.index(0,0), SalesView::QuantityRole).toInt(), 3);
        QCOMPARE(model.totalFormatted(), QString("37.50"));

        // Test Removal
        model.removeItem(0);
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.totalFormatted(), QString("0.00"));
    }

    // --- INTEGRATION: DB PERSISTENCE ---
    void testFullPersistenceCycle() {
        SalesView model;
        model.addItemToOrder(1);

        // Capture the ID before making the order (or from the model after)
        // Assuming you've updated SalesView to expose the ID of the pending order
        // Or check the database for the last inserted record.

        QVERIFY(model.makeOrder());

        // Since we use UUIDs, we can't guess "1".
        // For testing purposes, you might want to query the DB for the most recent UUID
        QSqlQuery query("SELECT id FROM orders ORDER BY created_at DESC LIMIT 1");
        QVERIFY(query.exec() && query.next());
        QString latestUuid = query.value(0).toString();

        // Pass the QString latestUuid instead of the int 1
        Order dbOrder = DatabaseManager::instance().loadOrder(latestUuid);

        QVERIFY(!dbOrder.items.isEmpty());
        QCOMPARE(dbOrder.items.first().menuItemId, 1);
    }

    void testMakeOrderSignalFlow() {
        SalesView model;
        QSignalSpy totalSpy(&model, &SalesView::totalChanged);

        // Signal 1: addItemToOrder calls calculateTotal()
        model.addItemToOrder(1);
        QCOMPARE(totalSpy.count(), 1);

        // Signal 2: makeOrder calls clearOrder, which calls calculateTotal()
        QVERIFY(model.makeOrder());

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(totalSpy.count(), 2);
    }
};

QTEST_MAIN(RestaurantPOSTest)
#include "tst_restaurantpos.moc"

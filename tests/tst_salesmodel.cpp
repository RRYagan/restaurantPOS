#include <QtTest>
#include <QSignalSpy>
#include "databasemanager.h"
#include "salesmodel.h"

class SalesModelTest : public QObject {
    Q_OBJECT

private slots:
    // Run once before all tests
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

    // --- UNIT TESTS ---

    void testAddItemIncrementsRows() {
        SalesModel model;
        QCOMPARE(model.rowCount(), 0);

        // Adding a new item (ID 1: Classic Cheeseburger)
        model.addItemToOrder(1);
        QCOMPARE(model.rowCount(), 1);

        // Verify data at row 0
        QModelIndex idx = model.index(0, 0);
        QCOMPARE(model.data(idx, SalesModel::QuantityRole).toInt(), 1);
        QCOMPARE(model.data(idx, SalesModel::NameRole).toString(), QString("Classic Cheeseburger"));
    }

    void testAddItemIncrementsQuantity() {
        SalesModel model;

        // Add same item twice
        model.addItemToOrder(1);
        model.addItemToOrder(1);

        // Row count stays 1, but quantity increases
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0,0), SalesModel::QuantityRole).toInt(), 2);
    }

    void testUpdateQuantityAndTotal() {
        SalesModel model;
        QSignalSpy spy(&model, &SalesModel::totalChanged);

        model.addItemToOrder(1); // Price 12.50
        model.updateQuantity(0, 3); // Total should be 37.50

        QCOMPARE(model.data(model.index(0,0), SalesModel::QuantityRole).toInt(), 3);
        QCOMPARE(model.totalFormatted(), QString("37.50"));
        QVERIFY(spy.count() >= 2); // Once for add, once for update
    }

    void testRemoveItem() {
        SalesModel model;
        model.addItemToOrder(1);
        model.addItemToOrder(2);
        QCOMPARE(model.rowCount(), 2);

        model.removeItem(0);
        QCOMPARE(model.rowCount(), 1);
        // Verify remaining item is index 0
        QCOMPARE(model.totalFormatted(), QString("4.50")); // Price of Fresh Lemonade
    }

    void testClearOrder() {
        SalesModel model;
        model.addItemToOrder(1);

        model.clearOrder();

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.totalFormatted(), QString("0.00"));
    }

    void testCartStatePersistence() {
        SalesModel model;

        // 1. Setup: Add an item to the "Active Cart"
        model.addItemToOrder(1);
        int originalCount = model.rowCount();
        QVERIFY(originalCount > 0);

        // 2. Transition: "View" a historical order
        // We pass an empty UUID string or a dummy UUID
        QString dummyUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        // model.viewOrderDetails(dummyUuid);

        // Verify current view is cleared (assuming dummyUuid has no items in DB)
        QCOMPARE(model.rowCount(), 0);

        // 3. Restoration: Switch back to the active user cart
        // model.switchToCart();

        // Verify the original items are back
        QCOMPARE(model.rowCount(), originalCount);
    }
};

QTEST_MAIN(SalesModelTest)
#include "tst_salesmodel.moc"

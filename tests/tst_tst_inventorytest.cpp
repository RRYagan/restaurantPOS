#include "tst_inventorytest.h"

void InventoryTest::initTestCase() {
    // Setup a temporary in-memory database for testing
    auto& db = DatabaseManager::instance();
    QVERIFY(db.openDatabase(":memory:"));
}

void InventoryTest::cleanupTestCase() {
    DatabaseManager::instance().closeDatabase();
}

void InventoryTest::init() {
    // Clear the table before each test to ensure isolation
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("DELETE FROM inventory");
}

void InventoryTest::testCreateItem() {
    InventoryModel model;
    QVariantMap item;
    item["name"] = "Test Product";
    item["packagesAvailable"] = 10;
    item["packagingUnitId"] = "box";
    item["quantityPerPackage"] = 5.0;
    item["quantityAvailable"] = 50.0;
    item["quantityUnitId"] = "kg";

    QVERIFY(model.createItem(item));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), InventoryModel::NameRole).toString(), "Test Product");
}

void InventoryTest::testReadItem() {
    InventoryModel model;
    QVariantMap item;
    item["name"] = "Read Test";
    model.createItem(item);

    InventoryItem result = model.inventoryAt(0);
    QCOMPARE(result.name, QString("Read Test"));
    QVERIFY(!result.id.isEmpty()); // UUID should have been generated
}

void InventoryTest::testUpdateItem() {
    InventoryModel model;
    model.createItem({{"name", "Original"}});

    QString id = model.inventoryAt(0).id;
    QVariantMap updateData;
    updateData["id"] = id;
    updateData["name"] = "Updated Name";
    updateData["packagesAvailable"] = 5;

    QVERIFY(model.updateItem(updateData));
    QCOMPARE(model.inventoryAt(0).name, QString("Updated Name"));
}

void InventoryTest::testRemoveItem() {
    InventoryModel model;
    model.createItem({{"name", "To Delete"}});
    QString id = model.inventoryAt(0).id;

    QVERIFY(model.removeItem(id));
    QCOMPARE(model.rowCount(), 0);
}

QUICK_TEST_MAIN(InventoryTest)
#include "tst_inventorytest.moc"

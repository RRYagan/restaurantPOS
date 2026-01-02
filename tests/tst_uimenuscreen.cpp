#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>
#include "databasemanager.h"
#include "../src/ui/sales/salesmodel.h"
#include "menumodel.h"

class MenuScreenUITest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Use in-memory DB so tests are fast and isolated
        DatabaseManager::instance().openDatabase(":memory:");

        // Register the C++ types that the QML engine expects to see
        // qmlRegisterType<MenuModel>("POS.Menu", 1, 0, "MenuModel");
        // qmlRegisterType<SalesModel>("POS.Sales", 1, 0, "SalesModel");
    }

    void cleanupTestCase() {
        DatabaseManager::instance().closeDatabase();
    }

    void testMakeOrderButtonEnabling() {
        QQuickView view;

        // 1. Tell the engine WHERE to look for local QML files (like Card.qml)
        // Adjust the number of "../" based on where your test binary sits relative to src
        view.engine()->addImportPath("../../../src/ui/sales");

        // 2. Load the file
        QUrl sourceUrl = QUrl::fromLocalFile("../../../src/ui/sales/MenuScreen.qml");
        view.setSource(sourceUrl);

        // 3. Debugging: If it fails, print the EXACT QML errors to the console
        if (view.status() != QQuickView::Ready) {
            for (const auto& error : view.errors()) {
                qDebug() << "QML Error:" << error.toString();
            }
        }

        QVERIFY2(view.status() == QQuickView::Ready, "QML failed to load. Check qDebug output for errors.");

        // In your test function after QVERIFY(view.status() == QQuickView::Ready)
        SalesModel* testModel = new SalesModel(&view);
        view.rootObject()->setProperty("salesModel", QVariant::fromValue(testModel));

        // Now test the logic
        QQuickItem* btn = view.rootObject()->findChild<QQuickItem*>("makeOrderButton");
        QCOMPARE(btn->property("enabled").toBool(), false); // Empty model [cite: 45, 48]

        testModel->addItemToOrder(1);
        QCOMPARE(btn->property("enabled").toBool(), true); // Model has items
    }
};

QTEST_MAIN(MenuScreenUITest)
#include "tst_uimenuscreen.moc"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QDebug>
#include <QQuickStyle>
#include <QIcon>
#include <QResource>
#include <QDir>
#include <QQmlContext>

// Include your manager
#include "databasemanager.h"
#include "lookupmodel.h"
#include "payment_enums.h"

// This macro is required for static linking of QML modules
Q_IMPORT_QML_PLUGIN(POS_UIPlugin)

// Helper function to turn "table_name" into "tableNameModel"
QString formatPropertyName(QString name) {
    QStringList parts = name.split('_');
    for (int i = 1; i < parts.size(); ++i) {
        parts[i][0] = parts[i][0].toUpper();
    }
    return parts.join("") + "Model";
}

int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);

    // KDBoat aesthetics require the Basic style for full custom control
    QQuickStyle::setStyle("Basic");

    app.setOrganizationName("Plasteq");
    app.setApplicationName("Hoteli Plus");

    qDebug() << QResource::registerResource(":/");
    QDir dir(":/sql/migrations");
    qDebug() << dir.entryList();

    if (!DatabaseManager::instance().openDatabase()) {
        qCritical() << "Could not open or initialize the database. Exiting...";
        return -1;
    }
    qmlRegisterUncreatableMetaObject(
        PaymentStatus::staticMetaObject,
        "com.plasteq.pos.payment",
        1, 0,
        "PaymentStatus", // This is the name used in QML
        "Accessing enums only"
        );
    QQmlApplicationEngine engine;

    // 1. List of your lookup tables from the SQL schema
    QStringList lookupTables = {
        "tax_classification", "product_type", "stock_movement_category",
        "purchase_receipt_type", "payment_method", "sales_receipt_type",
        "transaction_type", "quantity_unit", "packaging_unit",
        "currency", "country","product_category"
    };

    // 2. Loop and register each one safely
    for (const QString &tableName : lookupTables) {
        // Create the model instance
        LookupModel *model = new LookupModel(tableName, &app);
        model->select();

        // Format name: e.g., "tax_classification" -> "taxClassificationModel"
        // QString propertyName = formatPropertyName(tableName);
        QString propertyName = "_" + formatPropertyName(tableName);

        // Register to QML context
        engine.rootContext()->setContextProperty(propertyName, model);

        qDebug() << "Registered QML property:" << propertyName << "for table:" << tableName;
    }

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);


    engine.addImportPath(app.applicationDirPath() + "/qml");

    engine.loadFromModule("POS.UI", "Main");

    return app.exec();
}

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QDebug>

// Include your manager
#include "databasemanager.h"

// This macro is required for static linking of QML modules
Q_IMPORT_QML_PLUGIN(POS_SalesPlugin)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1. Initialize Database
    // We do this before the engine loads so data is ready for the UI

    if (!DatabaseManager::instance().openDatabase()) {
        qCritical() << "Could not open or initialize the database. Exiting...";
        return -1;
    }

    QQmlApplicationEngine engine;

    // Standard error handling for QML loading
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // qmlRegisterType<MenuModel>("POS.Sales", 1, 0, "MenuModel");

    engine.addImportPath(app.applicationDirPath() + "/qml");

    engine.loadFromModule("POS.Sales", "Main");

    return app.exec();
}

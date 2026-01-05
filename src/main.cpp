#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QDebug>
#include <QQuickStyle>
#include <QIcon>

// Include your manager
#include "databasemanager.h"

// This macro is required for static linking of QML modules
Q_IMPORT_QML_PLUGIN(POS_UIPlugin)

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);

    // KDBoat aesthetics require the Basic style for full custom control
    QQuickStyle::setStyle("Basic");

    app.setOrganizationName("Plasteq");
    app.setApplicationName("Hoteli Plus");

    if (!DatabaseManager::instance().openDatabase()) {
        qCritical() << "Could not open or initialize the database. Exiting...";
        return -1;
    }

    QQmlApplicationEngine engine;

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

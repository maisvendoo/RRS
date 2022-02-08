#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QScreen>
#include <QIcon>

#include "models/RoutesModel.h"
#include "models/TrainsModel.h"
#include "models/WaypointModel.h"
#include "SimParameters.h"
#include "RunGuard.h"
#include "SimRun.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    app.setWindowIcon(QIcon("qrc:/images/RRS_logo.ico"));

    RunGuard guard("Russian Railway Simulator");
    if (!guard.tryToRun()){
        engine.load(QUrl(QStringLiteral("qrc:/qml/ApplicationIsRunning.qml")));
        return app.exec();
    }

    QTranslator translator;
    if(translator.load(":/translation/translations/launcher.ru_RU.qm"))
        app.installTranslator(&translator);

    const QSize size = qApp->primaryScreen()->geometry().size();

    qmlRegisterType<RoutesModel>("RoutesModel", 1, 0, "RoutesModel");
    qmlRegisterType<TrainsModel>("TrainsModel", 1, 0, "TrainsModel");
    qmlRegisterType<WaypointModel>("WaypointModel", 1, 0, "WaypointModel");

    SimParameters simParameters;
    QQmlEngine::setObjectOwnership(&simParameters, QQmlEngine::CppOwnership);
    engine.rootContext()->setContextProperty("simParameters", &simParameters);

    SimRun run(&simParameters);
    QQmlEngine::setObjectOwnership(&simParameters, QQmlEngine::CppOwnership);
    engine.rootContext()->setContextProperty("simRun", QVariant::fromValue(&run));

    engine.rootContext()->setContextProperty("desktopWidgetWidth", size.width());
    engine.rootContext()->setContextProperty("desktopWidgetHeight", size.height());
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    return app.exec();
}

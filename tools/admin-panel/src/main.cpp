//------------------------------------------------------------------------------
//
//  Admin panel main
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    <QApplication>
#include    <QFile>
#include    <QDebug>

#include    "MainWindow.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AdminPanel");
    app.setApplicationVersion("1.0.0");

    // Загрузка стилей
    QFile styleFile(":/resources/styles.qss");
    if (styleFile.exists() && styleFile.open(QFile::ReadOnly))
    {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }

    MainWindow window;
    window.show();

    return app.exec();
}
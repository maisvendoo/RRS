//------------------------------------------------------------------------------
//
//  Simulator client main
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Simulator client main
 *  \copyright SimulatorClient
 *  \date 2026
 */

#include    <QApplication>
#include    <QFile>
#include    <QDebug>

#include    "MainWindow.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SimulatorClient");
    app.setApplicationVersion("1.0.0");

    // Загрузка стилей (опционально)
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
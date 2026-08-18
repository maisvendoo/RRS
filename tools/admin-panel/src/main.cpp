//------------------------------------------------------------------------------
//
//  Admin panel main
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    <QApplication>
#include    <QFile>
#include    <QDir>
#include    <QDebug>

#include    "MainWindow.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AdminPanel");
    app.setApplicationVersion("1.0.0");

    // Загрузка общей темы оформления проекта (../themes/dark-jedy.qss)
    QString basePath = QCoreApplication::applicationDirPath();
    QDir baseDir(basePath);
    baseDir.cdUp();
    QString themePath = baseDir.absolutePath() + "/themes/dark-jedy.qss";

    QFile themeFile(themePath);
    if (themeFile.exists() && themeFile.open(QFile::ReadOnly))
    {
        QString style = QLatin1String(themeFile.readAll());
        app.setStyleSheet(style);
        themeFile.close();
    }
    else
    {
        qWarning() << "Theme file not found:" << themePath;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
//------------------------------------------------------------------------------
//
//  Admin panel main
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    <QApplication>
#include    <QFile>
#include    <QDebug>
#include    <QFileInfo>
#include    <QCoreApplication>
#include    <QDir>

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

    // Путь к конфигу: ../cfg/admin-panel.xml (на уровень выше от bin)
    QString basePath = QCoreApplication::applicationDirPath();  // .../bin
    QDir baseDir(basePath);
    baseDir.cdUp();  // Поднимаемся на уровень выше, в корневую папку
    
    QString configPath = baseDir.absolutePath() + "/cfg/admin-panel.xml";
    QFileInfo fileInfo(configPath);
    
    if (fileInfo.exists())
    {
        qDebug() << "Config found:" << configPath;
    }
    else
    {
        qWarning() << "Config not found:" << configPath;
        qWarning() << "Trying current directory...";
        
        // Пробуем в текущей директории
        configPath = QCoreApplication::applicationDirPath() + "/cfg/admin-panel.xml";
        if (QFile::exists(configPath))
        {
            qDebug() << "Config found in current dir:" << configPath;
        }
        else
        {
            qWarning() << "Config not found, using default values";
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
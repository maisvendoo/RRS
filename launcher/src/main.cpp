//------------------------------------------------------------------------------
//
//      Russian Railway Simulator (RRS) launcer
//      (c) maisvendoo, 17/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Russian Railway Simulator (RRS) launcer
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 17/12/2018
 */

#include    <filesystem.h>
#include    "mainwindow.h"
#include    <QApplication>
#include    <QTranslator>
#include    <crash-handler.h>

/*!
 * \fn
 * \brief Entry point
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string log_path = fs.combinePath(fs.getLogsDir(),
                                                "launcher-crash.log");
    setup_crash_handler(log_path.c_str());

    QApplication a(argc, argv);
    QTranslator translator;

    if (translator.load("launcher.ru_RU.qm", ":/translations/translations"))
    {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}

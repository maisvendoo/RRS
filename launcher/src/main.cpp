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
    setup_crash_handler("../logs/launcher-crash.log");

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

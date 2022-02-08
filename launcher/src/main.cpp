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

/*!
 * \fn
 * \brief Entry point
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;

    if (translator.load(QLocale(),
                        QLatin1String("launcher"),
                        QLatin1String("."),
                        QLatin1String(":/translations/translations"),
                        QLatin1String(".qm")))

        a.installTranslator(&translator);


    MainWindow w;
    w.show();

    return a.exec();
}

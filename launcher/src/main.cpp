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
    MainWindow w;
    w.show();

    return a.exec();
}

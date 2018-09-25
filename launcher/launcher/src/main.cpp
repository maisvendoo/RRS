//------------------------------------------------------------------------------
//
//      True Railway Simulator GUI
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief True Railway Simulator GUI
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#include    "main.h"

/*!
 * \fn
 * \brief Application entry point
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Create application object
    LauncherApp    app(argc, argv);    

    // Start loop to signals processing
    return app.exec();
}

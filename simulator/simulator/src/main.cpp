//------------------------------------------------------------------------------
//
//      Main train simulation program
//      (c) maisvendoo, 01/09/2018
//      Devloper: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Main train simulation program
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 01/09/2018
 */

#include    "main.h"
#include    "exceptions.h"
#include    "sim-journal.h"

/*!
 * \fn
 * \brief Program entry point
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    init_journal();
    register_handlers();


    AppCore app(argc, argv);

    if (!app.init())
        return -1;
    else
        return app.exec();
}

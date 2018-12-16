//------------------------------------------------------------------------------
//
//      Video 3D client for railway simulation systems
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Video 3D client for railway simulation systems
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#include    "main.h"

/*!
 * \fn
 * \brief Entry point
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    RouteViewer viewer(argc, argv);

    if (viewer.isReady())
        return viewer.run();

    return 0;
}



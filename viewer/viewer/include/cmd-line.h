//------------------------------------------------------------------------------
//
//      Command line options
//      (c) maisvendoo, 04/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Command line options
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 04/12/2018
 */

#ifndef     CMD_LINE_H
#define     CMD_LINE_H

#include    <string>
#include    "command-line.h"

/*!
 * \struct
 * \brief
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    option_t<std::string>     host_addr;
    option_t<int>             port;
    option_t<int>             width;
    option_t<int>             height;
    option_t<bool>            fullscreen;
};

#endif // CMD_LINE_H

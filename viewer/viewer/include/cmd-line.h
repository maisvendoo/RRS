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

/*!
 * \struct
 * \brief structure to command line option store
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
struct option_t
{
    T value;
    bool is_present;

    option_t()
    {
        is_present = false;
    }
};

/*!
 * \struct
 * \brief
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    option_t<std::string>     route_dir;
    option_t<std::string>     train_config;
    option_t<std::string>     host_addr;
    option_t<int>             port;
    option_t<int>             width;
    option_t<int>             height;
    option_t<bool>            fullscreen;
    option_t<bool>            localmode;
    option_t<std::string>     notify_level;
    option_t<int>             direction;
    option_t<int>             screen;
};

#endif // CMD_LINE_H

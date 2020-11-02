//------------------------------------------------------------------------------
//
//      Command line data structures
//      (c) maisvendoo, 20/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Command line data structures
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 20/12/2018
 */

#ifndef     CMD_LINE_H
#define     CMD_LINE_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum CmdLineParseResult
{
    RESULT_OK,
    RESULT_HELP,
    RESULT_VERSION,
    RESULT_ERROR
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_param_t
{
    std::string     key;
    std::string     value;

    cmd_param_t()
        : key("")
        , value("")
    {

    }
};

#endif // CMD_LINE_H

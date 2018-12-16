//------------------------------------------------------------------------------
//
//      Parsing of command line option
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Parsing of command line option
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#ifndef     COMMAND_LINE_PARSER_H
#define     COMMAND_LINE_PARSER_H

#include    <osg/ArgumentParser>

#include    <sstream>

#include    "cmd-line.h"
#include    "get-value.h"

/*!
 * \class
 * \brief Command line parser
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CommandLineParser
{
public:

    /// Constructor
    CommandLineParser(int argc, char *argv[]);

    /// Destructor
    virtual ~CommandLineParser();

    /// Get command line options
    cmd_line_t getCommadLine() const;

protected:

    cmd_line_t cmd_line;
};

#endif

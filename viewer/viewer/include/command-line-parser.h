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

/*!
 * \fn
 * \brief Get value from string
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
bool getValue(const std::string &str, T &value)
{
    std::istringstream ss(str);

    try
    {
        ss >> value;
    }
    catch (std::exception ex)
    {
        (void) ex;
        return false;
    }

    return true;
}

#endif

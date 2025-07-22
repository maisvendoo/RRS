//------------------------------------------------------------------------------
//
//      Command line parameters structures
//      (c) maisvendoo 01/09/2018
//      Devloper: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Command line parameters structures
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 01/09/2018
 */

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

/*!
 * \struct
 * \brief structure to command line option store
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
struct option_t final
{
    T value = T();
    bool is_present = false;
};

/*!
 * \enum
 * \brief Command line parser result codes
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum CommandLineParserResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequired,
    CommandLineHelpRequired
};

#endif // COMMAND_LINE_H

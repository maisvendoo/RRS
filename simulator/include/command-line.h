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

#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <QMetaType>
#include    <QString>

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct command_line_t
{
    /// Train configuration file name
    option_t<QString> train_config;
};

Q_DECLARE_METATYPE(command_line_t)

#endif // COMMAND_LINE_H

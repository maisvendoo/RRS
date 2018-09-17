//------------------------------------------------------------------------------
//
//      Main application class
//      (c) maisvendoo, 01/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Main application class
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 01/09/2018
 */

#ifndef     APP_H
#define     APP_H

#include    <QCoreApplication>
#include    <QCommandLineParser>

#include    "simulator-command-line.h"
#include    "model.h"



/*!
 * \class
 * \brief Main application class
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AppCore : QCoreApplication
{
    Q_OBJECT

public:

    /// Constructor
    explicit AppCore(int &argc, char **argv);
    /// Destructor
    virtual ~AppCore();

    /// Application initialization
    bool init();

    /// Overrided exec() method (start signals processing loop)
    int exec();

private:

    /// Command line parser
    QCommandLineParser  parser;
    /// Parsed command line parameters
    simulator_command_line_t      command_line;

    /// Train motion model
    Model               *model;

    /// Command line parsing
    CommandLineParesrResult parseCommandLine(QCommandLineParser &parser,
                                             simulator_command_line_t &command_line,
                                             QString &errorMessage);
};

#endif // APP_H

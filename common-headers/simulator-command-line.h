#ifndef		SIMULATOR_COMMAND_LINE
#define		SIMULATOR_COMMAND_LINE

#include    "command-line.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct command_line_t
{
    /// Train configuration file name
    option_t<QString>   train_config;
    /// Clear simulator log file
    option_t<bool>      clear_log;
    /// Debug print in simulation loop
    option_t<bool>      debug_print;
};

#endif // SIMULATOR_COMMAND_LINE

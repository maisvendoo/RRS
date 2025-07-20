#ifndef		SIMULATOR_COMMAND_LINE
#define		SIMULATOR_COMMAND_LINE

#include    "command-line.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_command_line_t
{
    /// Route directory
    option_t<QString>   route_dir;
    /// Train configuration file name
    option_t<std::vector<QString>>   train_config;
    /// Initial railway coordinate
    option_t<std::vector<double>>    init_coord;
    /// Initial direction
    option_t<std::vector<int>>       direction;
    /// Initial trajectory
    option_t<std::vector<QString>>   trajectory_name;
};

#endif // SIMULATOR_COMMAND_LINE

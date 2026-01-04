#ifndef SIMULATOR_COMMAND_LINE
#define SIMULATOR_COMMAND_LINE

#include "command-line.h"

#include <QString>

#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_command_line_t final
{
    /// Start date and time struct serialized to 64-bit value
    option_t<std::int64_t> start_datetime;
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
    /// Scenario name
    option_t<QString>   scenario;
};

#endif // SIMULATOR_COMMAND_LINE

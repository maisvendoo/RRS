#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct command_line_t
{
    QString     route_dir;
    QString     train_config;

    command_line_t()
        : route_dir("")
        , train_config("")
    {

    }
};

#endif // COMMAND_LINE_H

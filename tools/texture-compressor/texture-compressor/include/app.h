#ifndef     APP_H
#define     APP_H

#include    <cmdparser.hpp>
#include    <command-line.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Application
{
public:

    Application() = default;

    ~Application() = default;

    int run(int argc, char *argv[]);

private:

    command_line_t cmd_line;

    bool init(int argc, char *argv[]);

    void configure_parser(cli::Parser &parser);

    bool parse_command_line(cli::Parser &parser,
                            command_line_t &cmd_line);
};

#endif

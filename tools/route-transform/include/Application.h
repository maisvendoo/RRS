#ifndef     APPLICATION_H
#define     APPLICATION_H

#include    <cmdparser.hpp>
#include    <command-line.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Application
{
public:

    /// Разбор аругментов командной строки
    bool parse_args(int argc, char* argv[]);

    bool transform_route() {return false;}

private:

    cmd_line_t cmd_line;

    /// Настройка парсера командной строки
    void configure_parser(cli::Parser& parser);

    /// Разбор командной строки
    void parse_command_line(cli::Parser& parser, cmd_line_t& cmd_line);

    /// Проверка параметров из командной строки
    bool check_command_line(const cmd_line_t& cmd_line);
};

#endif // APPLICATION_H

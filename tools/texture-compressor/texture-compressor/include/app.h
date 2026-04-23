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
};

#endif

#ifndef     APPLICATION_H
#define     APPLICATION_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Application
{
public:

    /// Разбор аругментов командной строки
    bool parse_args(int argc, char* argv[]) {return false;}

    bool transform_route() {return false;}
};

#endif // APPLICATION_H

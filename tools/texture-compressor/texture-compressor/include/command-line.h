#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
struct option_t
{
    T       value;
    bool    is_present;

    option_t()
        : value(T())
        , is_present(false)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct command_line_t
{
    option_t<std::string> model_path;
    bool generate_mipmaps = false;
};

#endif

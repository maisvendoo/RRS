#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
struct option_t
{
private:


    bool    is_present;

public:

    T       value;

    option_t()
        : value(T())
        , is_present(false)
    {

    }

    T get()
    {
        if (is_present)
            return value;

        return T();
    }

    void operator=(T value)
    {
        if (value == T())
        {
            is_present = false;
            return;
        }

        this->value = value;
        is_present = true;
    }

    bool isPresent() const
    {
        return is_present;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    option_t<std::string> input_route_path;
    option_t<std::string> output_route_path;

    option_t<std::string> input_model_path;
    option_t<std::string> input_texture_path;

    option_t<std::string> output_model_path;
};

#endif // COMMAND_LINE_H

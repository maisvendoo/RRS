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
        : is_present(false)
        , value(T())
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

    option_t<bool> transform_map;
    option_t<bool> transform_topology;

    option_t<double> shift_x;
    option_t<double> shift_y;
    option_t<double> shift_z;
};

#endif // COMMAND_LINE_H

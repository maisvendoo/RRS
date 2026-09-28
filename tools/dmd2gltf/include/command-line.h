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
    option_t<bool> input_only_used_at_map;
    option_t<bool> input_lights_at_map;
    option_t<bool> input_compress_textures;
    option_t<bool> smooth;
    option_t<int> num_threads;

    option_t<std::string> output_route_path;


    option_t<std::string> input_model_path;
    option_t<std::string> input_texture_path;

    option_t<std::string> output_model_path;
};

#endif // COMMAND_LINE_H

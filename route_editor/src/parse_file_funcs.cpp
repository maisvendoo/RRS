#include "parse_file_funcs.h"

#include <cstdarg>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>

static bool to_float(const char* str, float* out)
{
    char* endptr;
    errno = 0;
    const float result = std::strtof(str, &endptr);
    if (errno == 0 && *endptr == '\0')
    {
        *out = result;
        return true;
    }
    else
    {
        return false;
    }
}

static bool to_double(const char* str, double* out)
{
    char* endptr;
    errno = 0;
    const double result = std::strtod(str, &endptr);
    if (errno == 0 && *endptr == '\0')
    {
        *out = result;
        return true;
    }
    else
    {
        return false;
    }
}

static void print_error(const char* filename, int line_num, const char* error,
    const char* line_begin, const char* line_end)
{
    std::fprintf(stderr, "%s:%d: error: %s\n    %.*s\n",
        filename, line_num, error,
        static_cast<int>(line_end - line_begin), line_begin);
}

struct ParseValue
{
    ParseValueType type;
    char* buffer;
    std::size_t buffer_length;
    std::size_t buffer_size;
    void* out;

    bool process(const char** error)
    {
        buffer[buffer_length] = '\0';
        buffer_length = 0;

        switch (type)
        {
            case PARSE_VALUE_TYPE_FLOAT:
            {
                *error = "failed to read float value";
                return to_float(buffer, reinterpret_cast<float*>(out));
            }
            case PARSE_VALUE_TYPE_DOUBLE:
            {
                *error = "failed to read double value";
                return to_double(buffer, reinterpret_cast<double*>(out));
            }
            default:
            {
                return true;
            }
        }
    }

    bool append_char(char ch, const char** error)
    {
        buffer[buffer_length] = ch;
        ++buffer_length;

        *error = "value is not fitting into buffer";
        return buffer_length == buffer_size;
    }
};

char* read_file_in_buffer(const char* filename, const char* modes)
{
    std::FILE* const file = std::fopen(filename, modes);
    if (!file)
    {
        std::fprintf(stderr, "Failed to open %s\n", filename);
        return nullptr;
    }

    std::fseek(file, 0, SEEK_END);
    const long buffer_length = std::ftell(file);
    std::rewind(file);

    char* const buffer = reinterpret_cast<char*>(
        std::malloc(buffer_length + 1));

    if (!buffer)
    {
        std::fprintf(stderr, "Failed to allocate memory "
            "for %s content\n", filename);

        std::fclose(file);
        return nullptr;
    }

    const std::size_t bytes_read = std::fread(buffer, 1, buffer_length, file);
    buffer[buffer_length] = '\0';

    std::fclose(file);

    if (bytes_read < static_cast<std::size_t>(buffer_length))
    {
        std::fprintf(stderr, "Failed to read %s\n", filename);
        std::free(buffer);
        return nullptr;
    }

    return buffer;
}

bool parse_file_line_by_line(const char* filename, const char* modes,
    const char* separators, std::function<void()> func, int argc, ...)
{
    char* const buffer = read_file_in_buffer(filename, modes);
    if (!buffer)
    {
        return false;
    }

    ParseValue* const parse_values = reinterpret_cast<ParseValue*>(
        std::malloc(sizeof(ParseValue) * argc));

    if (!parse_values)
    {
        std::fprintf(stderr, "Failed to allocate memory "
            "for %s parse values\n", filename);

        std::free(buffer);
        return false;
    }

    std::va_list args;
    va_start(args, argc);

    for (int i = 0; i < argc; ++i)
    {
        ParseValue* const parse_value = &parse_values[i];
        parse_value->type = va_arg(args, ParseValueType);
        parse_value->buffer = va_arg(args, char*);
        parse_value->buffer_length = va_arg(args, std::size_t);

        if (parse_value->type != PARSE_VALUE_TYPE_STRING)
        {
            parse_value->out = va_arg(args, void*);
        }
    }

    va_end(args);

    int curr_state = 0;
    int line_num = 1;
    int index;
    const char* ptr = buffer;
    const char* line_begin = ptr;
    const char* error;

    while (true)
    {
        if (*ptr == '\n' || *ptr == '\0')
        {
            if (curr_state == 2 * argc - 1)
            {
                index = (curr_state - 1) / 2;
                if (!parse_values[index].process(&error))
                {
                    print_error(filename, line_num, error, line_begin, ptr);
                    std::free(parse_values);
                    std::free(buffer);
                    return false;
                }
            }
            else if (curr_state != 0 && curr_state != 2 * argc)
            {
                print_error(filename, line_num, "wrong parse value count",
                    line_begin, ptr);

                std::free(parse_values);
                std::free(buffer);
                return false;
            }

            if (curr_state != 0)
            {
                func();
            }

            if (*ptr == '\0')
            {
                std::free(parse_values);
                std::free(buffer);
                return true;
            }

            curr_state = 0;
            ++line_num;
            line_begin = ptr + 1;
        }
        else
        {
            bool is_separator = false;
            for (const char* sep = separators; *sep != '\0'; ++sep)
            {
                if (*ptr == *sep)
                {
                    is_separator = true;
                    break;
                }
            }

            const bool state_is_even = (curr_state % 2 == 0);

            if (is_separator)
            {
                if (!state_is_even)
                {
                    index = (curr_state - 1) / 2;
                    if (!parse_values[index].process(&error))
                    {
                        print_error(filename, line_num, error, line_begin, ptr);
                        std::free(parse_values);
                        std::free(buffer);
                        return false;
                    }

                    ++curr_state;
                }
            }
            else
            {
                if (curr_state == 2 * argc)
                {
                    print_error(filename, line_num, "too many parse values"
                        "int line", line_begin, ptr);

                    std::free(parse_values);
                    std::free(buffer);
                    return false;
                }

                if (state_is_even)
                {
                    ++curr_state;
                }

                index = (curr_state - 1) / 2;
                if (parse_values[index].append_char(*ptr, &error))
                {
                    print_error(filename, line_num, error, line_begin, ptr);
                    std::free(parse_values);
                    std::free(buffer);
                    return false;
                }
            }
        }

        ++ptr;
    }
}

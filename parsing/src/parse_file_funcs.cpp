#include "parse_file_funcs.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <vector>

static bool to_float(const char* str, float* out)
{
    char* endptr;
    errno = 0;
    const float result = strtof(str, &endptr);
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
    const double result = strtod(str, &endptr);
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
    fprintf(stderr, "%s:%d: error: %s\n    %.*s\n", filename, line_num, error,
        static_cast<int>(line_end - line_begin), line_begin);
}

bool ParseField::process(const char** error)
{
    buf[buf_len] = '\0';
    buf_len = 0;

    switch (type)
    {
        case FLOAT:
        {
            *error = "failed to read float value";
            return to_float(buf, reinterpret_cast<float*>(out));
        }
        case DOUBLE:
        {
            *error = "failed to read double value";
            return to_double(buf, reinterpret_cast<double*>(out));
        }
        default:
        {
            return true;
        }
    }
}

bool ParseField::append_char(char ch, const char** error)
{
    buf[buf_len] = ch;
    ++buf_len;

    *error = "value is not fitting into buffer";
    return buf_len == buf_size;
}

ParseField ParseField::String(char* buf, size_t size)
{
    return {STRING, buf, 0, size, nullptr};
}

ParseField ParseField::Float(char* buf, size_t size, float* out)
{
    return {FLOAT, buf, 0, size, out};
}

ParseField ParseField::Double(char* buf, size_t size, double* out)
{
    return {DOUBLE, buf, 0, size, out};
}

char* read_file_in_buffer(const char* filename, const char* modes)
{
    FILE* const file = fopen(filename, modes);
    if (!file)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    const long buffer_length = ftell(file);
    rewind(file);

    char* const buf = reinterpret_cast<char*>(malloc(buffer_length + 1));
    if (!buf)
    {
        fprintf(stderr, "Failed to allocate memory for %s content\n", filename);
        fclose(file);
        return nullptr;
    }

    const size_t bytes_read = fread(buf, 1, buffer_length, file);
    buf[buffer_length] = '\0';

    fclose(file);

    if (bytes_read < static_cast<size_t>(buffer_length))
    {
        fprintf(stderr, "Failed to read %s\n", filename);
        free(buf);
        return nullptr;
    }

    return buf;
}

bool parse_file_line_by_line(const char* filename, const char* modes,
    const char* separators, std::function<void()> func,
    std::initializer_list<ParseField> fields
)
{
    char* const buf = read_file_in_buffer(filename, modes);
    if (!buf)
    {
        return false;
    }

    std::vector<ParseField> parse_fields(fields.begin(), fields.end());

    int curr_state = 0;
    int line_num = 1;
    int index;
    const char* ptr = buf;
    const char* line_begin = ptr;
    const char* error;

    const int args = static_cast<int>(fields.size());

    while (true)
    {
        if (*ptr == '\n' || *ptr == '\0')
        {
            if (curr_state == 2 * args - 1)
            {
                index = (curr_state - 1) / 2;
                if (!parse_fields[index].process(&error))
                {
                    print_error(filename, line_num, error, line_begin, ptr);
                    free(buf);
                    return false;
                }
            }
            else if (curr_state != 0 && curr_state != 2 * args)
            {
                print_error(filename, line_num, "wrong parse value count",
                    line_begin, ptr);

                if (*ptr == '\0')
                {
                    free(buf);
                    return true;
                }

                curr_state = 0;
                ++line_num;
                line_begin = ptr + 1;
                ++ptr;
                continue;

                // free(buffer);
                // return false;
            }

            if (curr_state != 0)
            {
                func();
            }

            if (*ptr == '\0')
            {
                free(buf);
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
                    if (!parse_fields[index].process(&error))
                    {
                        print_error(filename, line_num, error, line_begin, ptr);
                        free(buf);
                        return false;
                    }

                    ++curr_state;
                }
            }
            else
            {
                if (curr_state == 2 * args)
                {
                    print_error(filename, line_num, "too many parse values "
                        "in line", line_begin, ptr);

                    ++ptr;
                    continue;

                    // free(buffer);
                    // return false;
                }

                if (state_is_even)
                {
                    ++curr_state;
                }

                index = (curr_state - 1) / 2;
                if (parse_fields[index].append_char(*ptr, &error))
                {
                    print_error(filename, line_num, error, line_begin, ptr);
                    free(buf);
                    return false;
                }
            }
        }

        ++ptr;
    }
}

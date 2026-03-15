#ifndef EDITOR_PARSE_FILE_FUNCS_H
#define EDITOR_PARSE_FILE_FUNCS_H

#include <functional>

enum ParseValueType
{
    PARSE_VALUE_TYPE_STRING,
    PARSE_VALUE_TYPE_FLOAT,
    PARSE_VALUE_TYPE_DOUBLE
};

char* read_file_in_buffer(const char* filename, const char* modes);

bool parse_file_line_by_line(const char* filename, const char* modes,
    const char* separators, std::function<void()> func, int argc, ...);

#endif // EDITOR_PARSE_FILE_FUNCS_H

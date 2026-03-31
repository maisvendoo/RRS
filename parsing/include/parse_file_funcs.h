#ifndef EDITOR_PARSE_FILE_FUNCS_H
#define EDITOR_PARSE_FILE_FUNCS_H

#include <cstddef>
#include <functional>
#include <initializer_list>

struct ParseField
{
    enum Type
    {
        STRING,
        FLOAT,
        DOUBLE
    } type;

    char* buf;
    size_t buf_len = 0;
    size_t buf_size;
    void* out = nullptr;

    static ParseField String(char* buf, size_t size);
    static ParseField Float(char* buf, size_t size, float* out);
    static ParseField Double(char* buf, size_t size, double* out);

    bool process(const char** error);
    bool append_char(char ch, const char** error);
};

char* read_file_in_buffer(const char* filename, const char* modes);

bool parse_file_line_by_line(const char* filename, const char* modes,
    const char* separators, std::function<void()> func,
    std::initializer_list<ParseField> fields
);

#endif // EDITOR_PARSE_FILE_FUNCS_H

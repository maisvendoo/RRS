#include "parse_file_funcs.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>

struct ParseValue
{
    ParseValueType type;
    char* buffer;
    std::size_t buffer_length;
    std::size_t buffer_size;
    void* out_variable;
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

    char* const buffer = reinterpret_cast<char*>(std::malloc(buffer_length + 1));
    if (!buffer)
    {
        std::fprintf(stderr, "Failed to allocate memory for %s content\n", filename);
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

}

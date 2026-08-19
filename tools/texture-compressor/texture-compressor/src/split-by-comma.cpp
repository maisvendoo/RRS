#include    <split-by-comma.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::string_view> split_by_comma_view(std::string_view str)
{
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = str.find(',');

    while (end != std::string_view::npos)
    {
        tokens.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(',', start);
    }
    tokens.emplace_back(str.substr(start));
    return tokens;
}

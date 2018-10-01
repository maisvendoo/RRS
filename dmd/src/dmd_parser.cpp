#include    "dmd_parser.h"

#include    <iterator>
#include    <algorithm>
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;

    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::string> parse_line(const std::string &line)
{
    std::string tmp = delete_symbol(line, '\r');
    tmp = delete_symbol(tmp, '\t');

    char delimiter = ' ';
    std::vector<std::string> tokens;

    if (*(tmp.end() - 1) != delimiter)
        tmp += delimiter;

    size_t pos = 0;
    std::string token;

    while ( (pos = tmp.find(delimiter)) != std::string::npos)
    {
        token = tmp.substr(0, pos);
        tmp.erase(0, pos + 1);
        if (!token.empty())
            tokens.push_back(token);
    }

    return tokens;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float str_to_float(const std::string &str)
{
    float buff = 0.0f;

    std::istringstream iss(str);

    iss >> buff;

    return buff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int str_to_int(const std::string &str)
{
    int buff = 0;

    std::istringstream iss(str);

    iss >> buff;

    return buff;
}

#ifndef     DMD_PARSER_H
#define     DMD_PARSER_H

#include    <string>
#include    <vector>
#include    <sstream>

std::string delete_symbol(const std::string &str, char symbol);

std::vector<std::string> parse_line(const std::string &line);

float str_to_float(const std::string &str);

int str_to_int(const std::string &str);

#endif // DMD_PARSER_H

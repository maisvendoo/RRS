#ifndef CORE_STRING_FUNCS_H
#define CORE_STRING_FUNCS_H

#include <QString>

#include <string>
#include <string_view>

std::string           to_std_string(const char* str);
const std::string&    to_std_string(const std::string& str);
std::string           to_std_string(std::string_view str);
std::string           to_std_string(const QString& str);

QString               to_qstring(const char* str);
QString               to_qstring(const std::string& str);
QString               to_qstring(std::string_view str);
const QString&        to_qstring(const QString& str);

bool                  string_is_empty(const char* str);
bool                  string_is_empty(const std::string& str);
bool                  string_is_empty(std::string_view str);
bool                  string_is_empty(const QString& str);

#endif // CORE_STRING_FUNCS_H

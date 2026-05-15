#ifndef STRING_FUNCS_H
#define STRING_FUNCS_H

#include <QString>

#include <string>
#include <string_view>

std::string to_std_string(std::string_view str);
std::string to_std_string(const QString& str);

QString to_qstring(std::string_view str);
QString to_qstring(const QString& str);

bool string_is_empty(std::string_view str);
bool string_is_empty(const QString& str);

#endif // STRING_FUNCS_H

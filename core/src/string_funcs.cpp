#include "core/string_funcs.h"

#include <QString>

#include <cassert>
#include <string>
#include <string_view>

std::string to_std_string(const char* str)
{
    assert(str != nullptr);
    return std::string{str};
}

const std::string& to_std_string(const std::string& str)
{
    return str;
}

std::string to_std_string(std::string_view str)
{
    return std::string{str};
}

std::string to_std_string(const QString& str)
{
    return str.toStdString();
}

std::string to_std_string(bool value)
{
    return std::string{value ? "true" : "false"};
}

QString to_qstring(const char* str)
{
    assert(str != nullptr);
    return QString{str};
}

QString to_qstring(const std::string& str)
{
    return QString::fromStdString(str);
}

QString to_qstring(std::string_view str)
{
    return QString{str.data()};
}

const QString& to_qstring(const QString& str)
{
    return str;
}

QString to_qstring(bool value)
{
    return QString{value ? "true" : "false"};
}

bool string_is_empty(const char* str)
{
    assert(str != nullptr);
    return str[0] == '\0';
}

bool string_is_empty(const std::string& str)
{
    return str.empty();
}

bool string_is_empty(std::string_view str)
{
    return str.empty();
}

bool string_is_empty(const QString& str)
{
    return str.isEmpty();
}

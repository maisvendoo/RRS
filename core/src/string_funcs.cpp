#include "core/string_funcs.h"

#include <QString>

#include <string>
#include <string_view>

std::string to_std_string(std::string_view str)
{
    return std::string{str};
}

std::string to_std_string(const QString& str)
{
    return str.toStdString();
}

QString to_qstring(std::string_view str)
{
    return QString{str.data()};
}

QString to_qstring(const QString& str)
{
    return str;
}

bool string_is_empty(std::string_view str)
{
    return str.empty();
}

bool string_is_empty(const QString& str)
{
    return str.isEmpty();
}

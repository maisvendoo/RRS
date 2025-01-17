#ifndef VIEWER_CFG_READER_H
#define VIEWER_CFG_READER_H

#include <vsg/io/Logger.h>

#include <pugixml.hpp>

#include <sstream>
#include <string>

class ConfigReader
{
public:
    ConfigReader(const std::string& path);

    void setSection(const std::string& section);

    void getValue(const std::string& param, std::string& value);

    template <typename T>
    void getValue(const std::string& param, T& value);

private:
    std::string getStringValue(const std::string& param);

private:
    std::string path;
    pugi::xml_document doc;
    pugi::xml_node current_section;
};

template <typename T>
void ConfigReader::getValue(const std::string& param, T& value)
{
    std::string string_value = getStringValue(param);
    if (string_value.empty())
    {
        return;
    }

    std::istringstream stream(string_value);

    try
    {
        stream >> value;
    }
    catch (...)
    {
        vsg::Logger::instance()->error(
            std::string("Invalid value of param ")
            + param
            + " in section "
            + current_section.name()
            + " in config file "
            + path
        );

        throw 1;
    }
}

#endif // VIEWER_CFG_READER_H

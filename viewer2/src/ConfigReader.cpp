#include "ConfigReader.h"

#include <vsg/io/Logger.h>

#include <pugixml.hpp>

#include <string>

ConfigReader::ConfigReader(const std::string& path)
    : path(path)
    , doc()
    , current_section()
{
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result)
    {
        vsg::Logger::instance()->error(std::string("Failed to open config file ") + path);
        throw 1;
    }
}

void ConfigReader::setSection(const std::string& section)
{
    current_section = doc.child("Config").child(section.c_str());
    if (!current_section)
    {
        vsg::Logger::instance()->error(
            std::string("Failed to find section ")
            + section
            + " in config file "
            + path
        );

        throw 1;
    }
}

void ConfigReader::getValue(const std::string& param, std::string& value)
{
    std::string string_value = getStringValue(param);
    if (!string_value.empty())
    {
        value = string_value;
    }
}

std::string ConfigReader::getStringValue(const std::string& param)
{
    if (!current_section)
    {
        vsg::Logger::instance()->error(std::string("Invalid current section in config file ") + path);
        throw 1;
    }

    std::string string_value = current_section.child_value(param.c_str());
    if (string_value.empty())
    {
        vsg::Logger::instance()->warn(
            std::string("No param ")
            + param
            + " in section "
            + current_section.name()
            + " in config file "
            + path
        );

        // throw 1;
    }

    return string_value;
}

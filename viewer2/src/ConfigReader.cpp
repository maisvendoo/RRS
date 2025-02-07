#include "ConfigReader.h"
#include "Logger.h"

#include <pugixml.hpp>

#include <string>

ConfigReader::ConfigReader(const std::string& path)
    : path(path)
{
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result)
    {
        LOG_ERROR("Failed to open config file %s", path.c_str());
        throw 1;
    }

    config_section = doc.child("Config");
}

void ConfigReader::setSection(const std::string& section)
{
    current_section = doc.child("Config").child(section.c_str());
    if (!current_section)
    {
        LOG_ERROR("Failed to find section %s in config file %s", section.c_str(), path.c_str());

        throw 1;
    }
}

void ConfigReader::setSection(const pugi::xml_node& section)
{
    current_section = section;
}

void ConfigReader::getValue(const std::string& param, std::string& value)
{
    std::string string_value = getStringValue(param);
    if (!string_value.empty())
    {
        value = string_value;
    }
}

const pugi::xml_node& ConfigReader::getConfigSection() const
{
    return config_section;
}

std::string ConfigReader::getStringValue(const std::string& param)
{
    if (!current_section)
    {
        LOG_ERROR("Invalid current section in config file %s", path.c_str());
        throw 1;
    }

    std::string string_value = current_section.child_value(param.c_str());
    if (string_value.empty())
    {
        LOG_WARN(
            "No param \"%s\" in section \"%s\" in config file \"%s\"",
            param.c_str(),
            current_section.name(),
            path.c_str()
        );

        // throw 1;
    }

    return string_value;
}

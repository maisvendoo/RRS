#ifndef VIEWER_CFG_READER_H
#define VIEWER_CFG_READER_H

#include "Logger.h"

#include <pugixml.hpp>

#include <sstream>
#include <string>

class ConfigReader
{
public:
    ConfigReader(const std::string& path);

    void setSection(const std::string& section);
    void setSection(const pugi::xml_node& section);

    void getValue(const std::string& param, std::string& value);

    template <typename T>
    void getValue(const std::string& param, T& value);

    const pugi::xml_node& getConfigSection() const;

private:
    std::string getStringValue(const std::string& param);

private:
    std::string path;
    pugi::xml_document doc;
    pugi::xml_node config_section;
    pugi::xml_node current_section;
};

// TODO: repair
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
        LOG_ERROR(
            "Invalid value of param %s in section %s in config file %s",
            param.c_str(),
            current_section.name(),
            path.c_str()
        );

        throw 1;
    }
}

#endif // VIEWER_CFG_READER_H

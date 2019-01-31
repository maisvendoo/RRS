#ifndef		CONFIG_READER_H
#define		CONFIG_READER_H

#include    <osgDB/XmlParser>
#include    <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ConfigReader
{
public:

    ConfigReader();

    ConfigReader(const std::string &path);

    virtual ~ConfigReader();

    bool load(const std::string &path);

    bool isOpenned() const;

    osgDB::XmlNode *getConfigNode();

    osgDB::XmlNode *findSection(osgDB::XmlNode *node, const std::string &section);

    template<class T>
    bool getValue(const std::string &section, const std::string &param, T &value)
    {
        std::string contents = getStrValue(section, param);

        if (!contents.empty())
        {
            std::istringstream ss(contents);

            try
            {
                ss >> value;
            }
            catch (std::exception)
            {
                return false;
            }
        }

        return true;
    }

    bool getValue(const std::string &section, const std::string &param, std::string &value)
    {
        std::string contents = getStrValue(section, param);

        if (!contents.empty())
        {
            value = contents;
        }

        return true;
    }

protected:

    bool    is_opened;

    osg::ref_ptr<osgDB::XmlNode> root;

    std::string getStrValue(const std::string &section, const std::string &param);
};

#endif

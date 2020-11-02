#include    "config-reader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConfigReader::ConfigReader()
    : is_opened(false)
{
    root = nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConfigReader::ConfigReader(const std::string &path)
{
    root = nullptr;
    is_opened = load(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConfigReader::~ConfigReader()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ConfigReader::load(const std::string &path)
{
    is_opened = false;

    root = osgDB::readXmlFile(path);

    is_opened = root.valid();

    return is_opened;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ConfigReader::isOpenned() const
{
    return is_opened;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::XmlNode *ConfigReader::getConfigNode()
{
    return findSection(root, "Config");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string ConfigReader::getStrValue(const std::string &section,
                                      const std::string &param)
{
    std::string contents = "";

    if (!root.valid())
        return contents;

    osg::ref_ptr<osgDB::XmlNode> cfgNode = findSection(root.get(), "Config");

    if (!cfgNode.valid())
        return contents;

    osg::ref_ptr<osgDB::XmlNode> secNode = findSection(cfgNode.get(), section);

    if (!secNode.valid())
        return contents;

    osg::ref_ptr<osgDB::XmlNode> valueNode = findSection(secNode.get(), param);

    if (!valueNode.valid())
        return contents;

    return valueNode->contents;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgDB::XmlNode *ConfigReader::findSection(osgDB::XmlNode *node, const std::string &section)
{
    osg::ref_ptr<osgDB::XmlNode> secNode;

    for (auto it = node->children.begin(); it != node->children.end(); ++it)
    {
        if ((*it)->name == section)
        {
            secNode = (*it);
            break;
        }
    }

    return secNode.release();
}


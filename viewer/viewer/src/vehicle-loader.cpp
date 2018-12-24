#include    "vehicle-loader.h"

#include    "config-reader.h"
#include    "filesystem.h"

#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ReadFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *loadVehicle(const std::string &configPath)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), configPath + fs.separator() + configPath + ".xml");
    ConfigReader cfg(cfg_path);

    std::string modelName = "";
    std::string textureName = "";

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "ExtModelName", modelName);
        cfg.getValue(secName, "ExtTextureName", textureName);
    }

    std::string modelPath = osgDB::findDataFile(fs.combinePath(fs.getVehicleModelsDir(), modelName));

    osg::ref_ptr<osg::Group> group = new osg::Group;

    if (!modelPath.empty())
    {
        modelPath = fs.toNativeSeparators(modelPath);
        osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(modelPath);
        group->addChild(model.get());
    }

    return group.release();
}

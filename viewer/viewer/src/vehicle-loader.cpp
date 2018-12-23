#include    "vehicle-loader.h"

#include    "config-reader.h"
#include    "filesystem.h"

#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ReadFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::MatrixTransform *loadVehicle(const std::string &configPath)
{
    ConfigReader cfg(configPath);

    std::string modelName = "";
    std::string textureName = "";

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "ExtModelName", modelName);
        cfg.getValue(secName, "ExtTextureName", textureName);
    }

    FileSystem &fs = FileSystem::getInstance();
    std::string modelPath = osgDB::findDataFile(fs.combinePath(fs.getVehicleModelsDir(), modelName));

    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

    if (!modelPath.empty())
    {
        modelPath = fs.toNativeSeparators(modelPath);
        osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(modelPath);
        transform->addChild(model.get());
    }

    return transform.release();
}

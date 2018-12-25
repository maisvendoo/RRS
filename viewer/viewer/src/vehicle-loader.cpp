//------------------------------------------------------------------------------
//
//      Functions for loading external model of vehicle
//      (c) maisvendoo, 24/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Functions for loading external model of vehicle
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 24/12/2018
 */

#include    "vehicle-loader.h"

#include    "config-reader.h"
#include    "filesystem.h"

#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <osgDB/ReadFile>
#include    <osg/Texture2D>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Texture2D *createTexture(const std::string &path)
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;

    // Loading texture from file
    std::string fileName = osgDB::findDataFile(path);

    if (fileName.empty())
    {
        OSG_FATAL << "ERROR: texture image " << path << " is't found" << std::endl;
        return nullptr;
    }

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(fileName);

    if (!image.valid())
        return nullptr;

    std::string ext = osgDB::getLowerCaseFileExtension(fileName);

    if (ext == "tga")
    {
        image->flipVertical();
    }

    texture->setImage(image.get());

    // Linear texture filtration (temporary!!!)
    texture->setNumMipmapLevels(0);
    texture->setFilter(osg::Texture::MIN_FILTER , osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER , osg::Texture::LINEAR);

    texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::REPEAT);
    texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::REPEAT);
    texture->setUnRefImageDataAfterApply(true);

    return texture.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *loadVehicle(const std::string &configPath)
{
    // Group node for vehicle model loading
    osg::ref_ptr<osg::Group> group = new osg::Group;

    // Open vehicle config file
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_config_path = configPath + fs.separator() + configPath + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);
    ConfigReader cfg(cfg_path);

    std::string modelName = "";
    std::string textureName = "";    

    // Reading data about body's 3D-model and texture
    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "ExtModelName", modelName);
        cfg.getValue(secName, "ExtTextureName", textureName);
    }

    // Loading 3D-model from file
    std::string model_path = fs.combinePath(fs.getVehicleModelsDir(), modelName);
    std::string modelPath = osgDB::findDataFile(model_path);

    osg::ref_ptr<osg::Node> model;

    if (!modelPath.empty())
    {
        modelPath = fs.toNativeSeparators(modelPath);
        model = osgDB::readNodeFile(modelPath);
        group->addChild(model.get());
    }
    else
    {
        OSG_FATAL << "ERROR: model " << model_path << " is't found";
    }

    if (!textureName.empty())
    {
        std::string tex_path = fs.combinePath(fs.getVehicleTexturesDir(), textureName);
        osg::ref_ptr<osg::Texture2D> texture = createTexture(tex_path);

        if (texture.valid())
        {
            model->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());

            std::string ext = osgDB::getLowerCaseFileExtension(tex_path);

            if (ext == "tga")
                model->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            else
                model->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        }
    }

    return group.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::MatrixTransform *loadWheels(const std::string &configPath)
{
    // Transformation for wheels own rotation (around own axis)
    osg::ref_ptr<osg::MatrixTransform> rotate = new osg::MatrixTransform;

    // Loading vehicle config file
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_config_path = configPath + fs.separator() + configPath + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);
    ConfigReader cfg(cfg_path);

    // Reading wheel parameters
    std::string wheelModelName = "";
    std::string wheelTextureName = "";

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "WheelModel", wheelModelName);
        cfg.getValue(secName, "WheelTexture", wheelTextureName);
    }

    // Loading wheels 3D-model
    std::string model_path = fs.combinePath(fs.getVehicleModelsDir(), wheelModelName);
    std::string wheelModelPath = osgDB::findDataFile(model_path);

    osg::ref_ptr<osg::Node> model;

    if (!wheelModelName.empty())
    {
        wheelModelPath = fs.toNativeSeparators(wheelModelPath);
        model = osgDB::readNodeFile(wheelModelPath);
        rotate->addChild(model.get());
    }
    else
    {
        OSG_FATAL << "ERROR: model " << model_path << " is't found";
    }

    // Loading texture
    if (!wheelTextureName.empty())
    {
        std::string tex_path = fs.combinePath(fs.getVehicleTexturesDir(),
                                              wheelTextureName);

        osg::ref_ptr<osg::Texture2D> texture = createTexture(tex_path);

        if (texture.valid())
        {
            model->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());

            std::string ext = osgDB::getLowerCaseFileExtension(tex_path);

            if (ext == "tga")
                model->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            else
                model->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        }
    }

    return rotate.release();
}

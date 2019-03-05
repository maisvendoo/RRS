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
#include    <osg/Material>
#include    <osgUtil/SmoothingVisitor>

#include    "model-smooth.h"
#include    "texture-filtering.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Node *loadModel(const std::string &modelName)
{
    FileSystem &fs = FileSystem::getInstance();

    osg::ref_ptr<osg::Node> model;

    // Loading 3D-model from file
    std::string model_path = fs.combinePath(fs.getVehicleModelsDir(), modelName);
    std::string modelPath = osgDB::findDataFile(model_path);

    if (!modelPath.empty())
    {
        modelPath = fs.toNativeSeparators(modelPath);
        model = osgDB::readNodeFile(modelPath);
    }
    else
    {
        OSG_FATAL << "ERROR: model " << model_path << " is't found";
        return nullptr;
    }

    ModelSmoother  smoother;
    model->accept(smoother);

    ModelTextureFilter texfilter;
    model->accept(texfilter);

    return model.release();
}

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
void applyTexture(osg::Node *model, const std::string &textureName)
{
    if (textureName.empty())
        return;

    FileSystem &fs = FileSystem::getInstance();

    std::string tex_path = fs.combinePath(fs.getVehicleModelsDir(), textureName);
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
    std::string modelShift = "";

    osg::Vec3 shift(0.0, 0.0, 0.0);

    // Reading data about body's 3D-model and texture
    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "ExtModelName", modelName);
        cfg.getValue(secName, "ExtTextureName", textureName);

        if (cfg.getValue(secName, "ModelShift", modelShift))
        {
            std::istringstream ss(modelShift);
            ss >> shift.x() >> shift.y() >> shift.z();
        }
    }

    osg::ref_ptr<osg::MatrixTransform> transShift = new osg::MatrixTransform(osg::Matrix::translate(shift));
    osg::ref_ptr<osg::Node> model = loadModel(modelName);

    if (model.valid())
    {
        transShift->addChild(model.get());
    }

    group->addChild(transShift.get());

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


    osg::ref_ptr<osg::Node> model = loadModel(wheelModelName);

    if (model.valid())
        rotate->addChild(model.get());

    return rotate.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void setAxis(osg::Group *vehicle, osg::MatrixTransform *wheel,
             const std::string &config_name)
{
    // Calculate vehicle config path
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = config_name + fs.separator() + config_name + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (!cfg.isOpenned())
    {
        OSG_FATAL << "Vehicle config " << cfg_path << " is't foung" << std::endl;
        return;
    }

    osgDB::XmlNode *config_node = cfg.getConfigNode();

    for (auto i = config_node->children.begin(); i != config_node->children.end(); ++i)
    {
        if ((*i)->name == "Vehicle")
        {
            osgDB::XmlNode *vehicle_node = *i;

            // Read all "Axis" parameters in config
            for (auto j = vehicle_node->children.begin(); j != vehicle_node->children.end(); ++j)
            {
                if ((*j)->name == "Axis")
                {
                    std::string tmp = (*j)->contents;

                    // Parse coordinates of axis
                    std::istringstream ss(tmp);
                    osg::Vec3 pos;
                    ss >> pos.x() >> pos.y() >> pos.z();

                    // Shift axis transformation
                    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
                    trans->setMatrix(osg::Matrix::translate(pos));
                    trans->addChild(wheel);
                    // Add axis to vehicle model
                    vehicle->addChild(trans.get());
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loadCabine(osg::Group *vehicle,
                const std::string &config_name,
                osg::ref_ptr<osg::Node> &cabine_model)
{
    // Calculate vehicle config path
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = config_name + fs.separator() + config_name + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (!cfg.isOpenned())
    {
        OSG_FATAL << "Vehicle config " << cfg_path << " is't foung" << std::endl;
        return;
    }

    std::string cabineModelName = "";
    std::string cabineTextureName = "";
    std::string cabineShift = "";

    osg::Vec3 shift(0.0, 0.0, 0.0);

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "CabineModel", cabineModelName);
        cfg.getValue(secName, "CabineTexture", cabineTextureName);

        if (cabineModelName.empty())
            return;

        if (cfg.getValue(secName, "CabineShift", cabineShift))
        {
            std::istringstream ss(cabineShift);
            ss >> shift.x() >> shift.y() >> shift.z();
        }
    }

    osg::ref_ptr<osg::MatrixTransform> transShift = new osg::MatrixTransform(osg::Matrix::translate(shift));
    cabine_model = loadModel(cabineModelName);

    if (cabine_model.valid())
        transShift->addChild(cabine_model.get());

    vehicle->addChild(transShift.get());    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float getLength(const std::string &config_name)
{
    float length = 16.0f;

    // Calculate vehicle config path
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_cfg_path = config_name + fs.separator() + config_name + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_cfg_path);

    // Load config file
    ConfigReader cfg(cfg_path);

    if (!cfg.isOpenned())
    {
        OSG_FATAL << "Vehicle config " << cfg_path << " is't foung" << std::endl;
        return length;
    }

    if (cfg.isOpenned())
    {
        std::string secName = "Vehicle";
        cfg.getValue(secName, "Length", length);
    }

    return length;
}

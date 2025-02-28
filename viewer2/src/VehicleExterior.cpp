#include    "VehicleExterior.h"

#include    <vsg/maths/transform.h>
#include    <vsg/io/read.h>

#include    "filesystem.h"
#include    "CfgReader.h"
#include    "AnimTransformVisitor.h"
#include    "ProcAnimation.h"
#include    "sound-manager.h"
#include    "Logger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::step(float t, float dt)
{
    if (animations.empty())
    {
        return;
    }

    for (auto animation : animations)
    {
        animation.second->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::loadVehicle(std::string &cfg_dir, std::string &cfg_file, SoundManager *sm)
{
    // Open vehicle config file
    FileSystem &fs = FileSystem::getInstance();
    std::string relative_config_path = cfg_dir + fs.separator() + cfg_file + ".xml";
    std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);

    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        LOG_WARN("Fail to open config file: %s", cfg_path.c_str());
        return false;
    }

    LOG_INFO("Opened config file: %s", cfg_path.c_str());
    QString sec_name = "Vehicle";

    // Reading data about body's 3D-model and texture
    QString modelName = "";
    QString textureName = "";
    QString modelShift = "";
    vsg::dvec3 shift(0.0, 0.0, 0.0);

    cfg.getString(sec_name, "ExtModelName", modelName);
    if (modelName.isEmpty())
    {
        LOG_WARN("Fail to read parameter <ExtModelName> in config file: %s", cfg_path.c_str());
        return false;
    }

    cfg.getString(sec_name, "ExtTexturesDir", textureName);
    auto model = loadModel(modelName.toStdString(), textureName.toStdString());
    if (!model)
        return false;

    if (cfg.getString(sec_name, "ModelShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
        model->matrix = vsg::translate(shift);
    }

    transform->addChild(model);

    // Reading data about cabine's 3D-model and texture
    modelName = "";
    textureName = "";
    modelShift = "";
    shift = {0.0, 0.0, 0.0};

    cfg.getString(sec_name, "CabineModel", modelName);
    if (!modelName.isEmpty())
    {
        cfg.getString(sec_name, "CabineTexturesDir", textureName);
        auto cabine = loadModel(modelName.toStdString(), textureName.toStdString());
        if (cabine)
        {
            if (cfg.getString(sec_name, "CabineShift", modelShift))
            {
                std::istringstream ss(modelShift.toStdString());
                ss >> shift.x >> shift.y >> shift.z;
                cabine->matrix = vsg::translate(shift);
            }

            transform->addChild(cabine);
        }
    }

    QString animationsDir = "";
    cfg.getString(sec_name, "AnimationsConfigDir", animationsDir);
    load_animations(animationsDir.toStdString());
    load_model_animations(animationsDir.toStdString());

    QString soundsDir = "";
    cfg.getString(sec_name, "SoundDir", soundsDir);
    load_sounds(soundsDir.toStdString(), sm);

    relative_config_path = cfg_dir + fs.separator() + "displays.xml";
    cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);
    load_displays(cfg_path);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::MatrixTransform> VehicleExterior::loadModel(const std::string &modelName, const std::string &textureName)
{
    (void) textureName; // TODO

    FileSystem &fs = FileSystem::getInstance();
    std::string model_path = fs.combinePath(fs.getVehicleModelsDir(), modelName);

    auto model_node = vsg::read_cast<vsg::Node>(model_path);

    if (model_node)
    {
        LOG_INFO("Loaded model from file: %s", model_path.c_str());
        vsg::ref_ptr<vsg::MatrixTransform> node = vsg::MatrixTransform::create();
        node->addChild(model_node);
        return node;
    }

    LOG_WARN("Fail to load model from file: %s", model_path.c_str());
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_animations(const std::string& animations_dir)
{
    int old_size = animations.size();
    AnimTransformVisitor atv(&animations, animations_dir);
    transform->accept(atv);
    LOG_INFO("Loaded %u custom animations", animations.size() - old_size);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_model_animations(const std::string &animations_dir)
{
    int old_size = animations.size();
    // TODO
    LOG_INFO("Loaded %u model animations", animations.size() - old_size);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_sounds(const std::string &sounds_dir, SoundManager *sm)
{
    sounds_id = sm->loadVehicleSounds(QString(sounds_dir.c_str()));
    LOG_INFO("Loaded %u sounds", sounds_id.size());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_displays(const std::string &cfg_path)
{

}

#include    "VehicleExterior.h"

#include <iostream>
#include    <vsg/maths/transform.h>
#include    <vsg/io/read.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>

#include "MyGui.h"
#include    "filesystem.h"
#include    "CfgReader.h"
#include    "AnimTransformVisitor.h"
#include    "ProcAnimation.h"
#include    "sound-manager.h"
#include    "Logger.h"
#include    "helper.h"

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
bool VehicleExterior::loadVehicle(std::string &cfg_dir, std::string &cfg_file, SoundManager *sm, vsg::ref_ptr<vsg::Options> options)
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
    auto model = loadModel(modelName.toStdString(), textureName.toStdString(), options);

    if (!model)
    {
        return false;
    }

    if (cfg.getString(sec_name, "ModelShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
        model->matrix = vsg::translate(shift);
    }

    model->setValue("name", "vehicle");
    transform->addChild(model);

    // Reading data about cabine's 3D-model and texture
    modelName = "";
    textureName = "";
    modelShift = "";
    shift = {0.0, 0.0, 0.0};

    cfg.getString(sec_name, "CabineModel", modelName);
    cfg.getString(sec_name, "CabineTexturesDir", textureName);

    if (!modelName.isEmpty())
    {
        cfg.getString(sec_name, "CabineTexturesDir", textureName);
        auto cabine = loadModel(modelName.toStdString(), textureName.toStdString(), options);
        if (cabine)
        {
            if (cfg.getString(sec_name, "CabineShift", modelShift))
            {
                std::istringstream ss(modelShift.toStdString());
                ss >> shift.x >> shift.y >> shift.z;
                cabine->matrix = vsg::translate(shift);
            }

            cabine->setValue("name", "cabine");
            transform->addChild(cabine);
            transform->setValue("name", "vehicle + cabine");
        }
    }
    else
    {
        transform->setValue("name", "only vehicle");
    }

    // Params::nodes.emplace_back(transform);

    modelShift = "";
    if (cfg.getString(sec_name, "DriverPos", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> driver_pos.x >> driver_pos.y >> driver_pos.z;
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

    print_node(transform);
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::MatrixTransform> VehicleExterior::loadModel(const std::string &modelName, const std::string &textureName, vsg::ref_ptr<vsg::Options> options)
{
    static std::map<std::string, vsg::ref_ptr<vsg::Node>> loaded_nodes;
    (void) textureName; // TODO

    FileSystem &fs = FileSystem::getInstance();
    std::string model_path = fs.combinePath(fs.getVehicleModelsDir(), modelName);

    vsg::ref_ptr<vsg::Node> model_node;

    if (loaded_nodes.count(model_path))
    {
        model_node = loaded_nodes[model_path];
    }
    else
    {
        model_node = vsg::read_cast<vsg::Node>(model_path, options);

        if (auto cull_node = vsg::ref_ptr(model_node->cast<vsg::CullNode>()))
        {
            if (auto mt = vsg::ref_ptr(cull_node->child->cast<vsg::MatrixTransform>()))
            {
                if (auto old_outer_group = vsg::ref_ptr(mt->children[0]->cast<vsg::Group>()))
                {
                    auto new_outer_group = vsg::Group::create();

                    for (auto& child : old_outer_group->children)
                    {
                        std::string name;
                        child->getValue("name", name);

                        auto transform = vsg::MatrixTransform::create();
                        transform->setValue("name", name);
                        transform->addChild(child);
                        new_outer_group->addChild(transform);
                    }

                    mt->children[0] = new_outer_group;
                }
            }
        }

        loaded_nodes.emplace(model_path, model_node);
    }

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
    AnimTransformVisitor atv(&animations, animations_dir, transform);
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

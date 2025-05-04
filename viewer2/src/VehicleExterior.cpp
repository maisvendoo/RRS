#include "VehicleExterior.h"

#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/threading/OperationThreads.h>

#include "CfgReader.h"
#include "filesystem.h"
#include "MyGui.h"
#include "LoadModelOperation.h"
#include "ProcAnimation.h"
#include "sound-manager.h"

#include <iostream>
#include <map>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::step(float t, float dt)
{
    for (auto& [signal_id, animation] : animations)
    {
        animation->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::loadVehicle(std::string& cfg_dir, std::string& cfg_file, SoundManager* sm, vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Options> options)
{
    // Open vehicle config file
    FileSystem& fs = FileSystem::getInstance();
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

    cfg.getString(sec_name, "ExtModelName", modelName);
    if (modelName.isEmpty())
    {
        LOG_WARN("Fail to read parameter <ExtModelName> in config file: %s", cfg_path.c_str());
        return false;
    }
    
    QString animationsDir = "";
    QString textureDir = "";
    QString soundsDir = "";
    QString modelShift = "";
    vsg::dvec3 shift(0.0, 0.0, 0.0);

    cfg.getString(sec_name, "AnimationsConfigDir", animationsDir);
    cfg.getString(sec_name, "ExtTexturesDir", textureDir);
    cfg.getString(sec_name, "SoundDir", soundsDir);
    if (cfg.getString(sec_name, "ModelShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
    }
/*
    auto model = loadModel(modelName.toStdString(), textureDir.toStdString(), options);
    if (!model)
    {
        return false;
    }
*/
    vsg::ref_ptr<vsg::MatrixTransform> vehicle_node = vsg::MatrixTransform::create();
    vehicle_node->matrix = vsg::translate(shift);
    vehicle_node->setValue("name", "vehicle");
    transform->addChild(vehicle_node);

    std::string model_filename_path = fs.combinePath(fs.getVehicleModelsDir(), modelName.toStdString());
    std::string animations_dir = animationsDir.toStdString();
    std::string textures_dir = textureDir.toStdString();
    std::string sounds_dir = soundsDir.toStdString();

    // Load model
    options->operationThreads->add(LoadModelOperation::create(viewer,
                                                              vehicle_node,
                                                              model_filename_path,
                                                              animations_dir,
                                                              textures_dir, // TODO
                                                              options,
                                                              animations));

    // Reading data about cabine's 3D-model and texture
    modelName = "";
    textureDir = "";
    modelShift = "";
    shift = {0.0, 0.0, 0.0};

    cfg.getString(sec_name, "CabineModel", modelName);

    if (!modelName.isEmpty())
    {
        cfg.getString(sec_name, "CabineTexturesDir", textureDir);
/*
        auto cabine = loadModel(modelName.toStdString(), textureDir.toStdString(), options);
        if (cabine)
        {
*/
            if (cfg.getString(sec_name, "CabineShift", modelShift))
            {
                std::istringstream ss(modelShift.toStdString());
                ss >> shift.x >> shift.y >> shift.z;
            }
            vsg::ref_ptr<vsg::MatrixTransform> cabine_node = vsg::MatrixTransform::create();
            cabine_node->matrix = vsg::translate(shift);
            cabine_node->setValue("name", "cabine");
            transform->addChild(cabine_node);
            transform->setValue("name", "vehicle + cabine");

            model_filename_path = fs.combinePath(fs.getVehicleModelsDir(), modelName.toStdString());

            // Load model
            options->operationThreads->add(LoadModelOperation::create(viewer,
                                                                      cabine_node,
                                                                      model_filename_path,
                                                                      animations_dir,
                                                                      textures_dir, // TODO
                                                                      options,
                                                                      animations));
/*
        }
        else
        {
            transform->setValue("name", "only vehicle");
        }
*/
    }
    else
    {
        transform->setValue("name", "only vehicle");
    }

    // GUIParams::nodes.emplace_back(transform);

    modelShift = "";
    if (cfg.getString(sec_name, "DriverPos", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> driver_pos.x >> driver_pos.y >> driver_pos.z;
    }
/*
    auto pdo = vsg::PropagateDynamicObjects::create();
    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    load_animations(animationsDir.toStdString(), options, pdo, duplicate);
    load_model_animations(animationsDir.toStdString());

    transform->traverse(*pdo);

    if (!pdo->dynamicObjects.empty())
    {
        for (auto& object : pdo->dynamicObjects)
        {
            if (!duplicate->contains(object))
            {
                duplicate->insert(object);
            }
        }

        transform->children[0] = copyop(model);

        if (transform->children.size() == 2)
        {
            transform->children[1] = copyop(vsg::ref_ptr(transform->children[1]->cast<vsg::MatrixTransform>()));
        }
    }
*/
    load_sounds(sounds_dir, sm);

    // TODO
    relative_config_path = cfg_dir + fs.separator() + "displays.xml";
    cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);
    load_displays(cfg_path);

    return true;
}
/*
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

        // if (auto cull_node = vsg::ref_ptr(model_node->cast<vsg::CullNode>()))
        // {
        //     if (auto mt = vsg::ref_ptr(cull_node->child->cast<vsg::MatrixTransform>()))
        //     {
        //         if (auto old_outer_group = vsg::ref_ptr(mt->children[0]->cast<vsg::Group>()))
        //         {
        //             auto new_outer_group = vsg::Group::create();

        //             for (auto& child : old_outer_group->children)
        //             {
        //                 std::string name;
        //                 child->getValue("name", name);

        //                 auto transform = vsg::MatrixTransform::create();
        //                 transform->setValue("name", name);
        //                 transform->addChild(child);
        //                 new_outer_group->addChild(transform);
        //             }

        //             mt->children[0] = new_outer_group;
        //         }
        //     }
        // }

        loaded_nodes.emplace(model_path, model_node);
    }

    if (model_node)
    {
        LOG_INFO("Loaded model from file: %s", model_path.c_str());
        vsg::ref_ptr<vsg::MatrixTransform> node = vsg::MatrixTransform::create();
        node->addChild(model_node);

        GUIParams::nodes.emplace_back(model_node);

        return node;
    }

    LOG_WARN("Fail to load model from file: %s", model_path.c_str());
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_animations(const std::string& animations_dir, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo, vsg::ref_ptr<vsg::Duplicate> duplicate)
{
    int old_size = animations.size();

    AnimTransformVisitorCreateInfo atv_create_info = {
        .pdo = pdo,
        .duplicate = duplicate,
        .animations_dir = animations_dir,
        .animations = &animations
    };

    AnimTransformVisitor atv(atv_create_info);
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
*/
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
// TODO
}

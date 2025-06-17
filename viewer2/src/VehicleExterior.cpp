#include "VehicleExterior.h"

#include "Logger.h"
#include "MyGui.h"
#include "filesystem.h"
#include "CfgReader.h"
#include "LoadModelOperation.h"
#include "ProcAnimation.h"
#include "sound-manager.h"
// #include "MyGui.h"

#include <iostream>
#include <qdom.h>
#include <sstream>
#include <string>
#include <vector>
#include <vsg/core/Array.h>
#include <vsg/maths/vec2.h>
#include <vsg/threading/OperationThreads.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::step(float t, float dt)
{
    for (auto& [signal_id, animation] : animations->animations)
    {
        animation->setSignals(&server_signals);
        animation->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::loadVehicle(const std::string& cfg_dir, const std::string& cfg_file, SoundManager* sm, vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Options> options)
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

    QString sec_name = "Vehicle";

    // Reading data about body's 3D-model
    QString modelName = "";

    cfg.getString(sec_name, "ExtModelName", modelName);
    if (modelName.isEmpty())
    {
        LOG_WARN("Fail to read parameter <ExtModelName> in config file: %s", cfg_path.c_str());
        return false;
    }

    QString animationsDir = "";
    QString soundsDir = "";
    QString modelShift = "";
    vsg::dvec3 shift(0.0, 0.0, 0.0);

    cfg.getString(sec_name, "AnimationsConfigDir", animationsDir);
    cfg.getString(sec_name, "SoundDir", soundsDir);
    if (cfg.getString(sec_name, "ModelShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
    }

    vsg::ref_ptr<vsg::MatrixTransform> vehicle_node = vsg::MatrixTransform::create();
    vehicle_node->matrix = vsg::translate(shift);
    vehicle_node->setValue("name", "vehicle");
    transform->addChild(vehicle_node);

    std::string model_filename_path = fs.combinePath(fs.getVehicleModelsDir(), modelName.toStdString());
    std::string animations_dir = animationsDir.toStdString();
    std::string sounds_dir = soundsDir.toStdString();

    // Load model
    options->operationThreads->add(LoadModelOperation::create(viewer,
                                                              vehicle_node,
                                                              model_filename_path,
                                                              animations_dir,
                                                              options,
                                                              animations));

    // Reading data about cabine's 3D-model
    modelName = "";
    modelShift = "";
    shift = {0.0, 0.0, 0.0};

    cfg.getString(sec_name, "CabineModel", modelName);

    if (!modelName.isEmpty())
    {
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
            options->operationThreads->add(LoadModelOperation::create(
                viewer,
                cabine_node,
                model_filename_path,
                animations_dir,
                options,
                animations
            ));
    }
    else
    {
        transform->setValue("name", "only vehicle");
    }

    GUIParams::nodes.emplace_back(transform);

    modelShift = "";
    if (cfg.getString(sec_name, "DriverPos", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> driver_pos.x >> driver_pos.y >> driver_pos.z;
    }

    load_sounds(sounds_dir, sm);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::load_sounds(const std::string &sounds_dir, SoundManager *sm)
{
    sounds_id = sm->loadVehicleSounds(QString(sounds_dir.c_str()));
    LOG_INFO("Loaded %u sounds", sounds_id.size());
}

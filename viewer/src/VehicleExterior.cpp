#include "VehicleExterior.h"

#include "AnimatedPagedLOD.h"
#include "CfgReader.h"
#include "filesystem.h"
#include "Logger.h"
#include "ProcAnimation.h"
#include "sound-manager.h"

#include <vsg/core/Array.h>
#include <vsg/maths/vec2.h>
#include <vsg/threading/OperationThreads.h>

#include <QDomNode>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::step(float t, float dt)
{
    for (const auto& animated_pagedLOD : animated_nodes)
    {
        if (animated_pagedLOD->children[0].node)
        {
            for (const auto& [signal_id, animation] : animated_pagedLOD->animations_map->animations)
            {
                animation->step(t, dt);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleExterior::step(float t, float dt, std::vector<float>* server_signals)
{
    for (const auto& animated_pagedLOD : animated_nodes)
    {
        if (animated_pagedLOD->children[0].node)
        {
            for (const auto& [signal_id, animation] : animated_pagedLOD->animations_map->animations)
            {
                animation->setSignals(server_signals);
                animation->step(t, dt);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::loadVehicle(const std::string& cfg_dir, const std::string& cfg_file, SoundManager* sm, vsg::ref_ptr<vsg::Options> options)
{
    // Open vehicle config file
    FileSystem& fs = FileSystem::getInstance();
    const std::string relative_config_path = cfg_dir + fs.separator() + cfg_file + ".xml";
    const std::string cfg_path = fs.combinePath(fs.getVehiclesDir(), relative_config_path);

    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        LOG_WARN("Fail to open config file: %s", cfg_path.c_str());
        return false;
    }

    // Camera positions in vehicle cabines
    load_cabine_positions(cfg_path, cfg);

    // Vehicle sounds
    load_sounds(cfg_path, cfg, sm);

    // Reading data about body's 3D-model
    load_body_model(cfg_path, cfg, options);

    // Reading data about cabine's 3D-model
    load_cabine_model(cfg_path, cfg, options);

    transform->setValue("name", cfg_file);
    return (transform->children.size() > 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::load_cabine_positions(const std::string &cfg_path, CfgReader &cfg)
{
    QDomNode secNode = cfg.getFirstSection("Cabine");
    if (secNode.isNull())
    {
        LOG_WARN("Fail to find section <Cabine> in config file: %s", cfg_path.c_str());
        return false;
    }

    driver_pos.clear();
    driver_dir.clear();

    while (true)
    {
        vsg::dvec3 dp = {0.0, 0.0, 0.0};
        QString DriverPos = "";
        if (cfg.getString(secNode, "DriverPos", DriverPos))
        {
            std::istringstream ss(DriverPos.toStdString());
            ss >> dp.x >> dp.y >> dp.z;
        }
        driver_pos.push_back(dp);

        double dd = 0.0;
        cfg.getDouble(secNode, "DriverDir", dd);
        driver_dir.push_back(vsg::radians(dd));

        secNode = cfg.getNextSection();
        if (secNode.isNull())
            return true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::load_sounds(const std::string &cfg_path, CfgReader &cfg, SoundManager* sm)
{
    QString soundsDir = "";
    cfg.getString("Vehicle", "SoundDir", soundsDir);
    if (soundsDir.isEmpty())
    {
        LOG_WARN("Fail to read parameter <SoundDir> in config file: %s", cfg_path.c_str());
        return false;
    }

    const std::string sounds_dir = soundsDir.toStdString();
    sounds_id = sm->loadVehicleSounds(sounds_dir);
    LOG_INFO("Loaded %u sounds from %s", sounds_id.size(), sounds_dir.c_str());
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::load_body_model(const std::string &cfg_path, CfgReader &cfg, vsg::ref_ptr<vsg::Options> options)
{
    const QString section_name = "Vehicle";
    QString modelName = "";

    cfg.getString(section_name, "ExtModelName", modelName);
    if (modelName.isEmpty())
    {
        LOG_WARN("Fail to read parameter <ExtModelName> in config file: %s", cfg_path.c_str());
        return false;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string model_filename_path = fs.combinePath(fs.getVehicleModelsDir(), modelName.toStdString());
    if (!vsg::fileExists(model_filename_path))
    {
        LOG_WARN("Fail to find file: %s", model_filename_path.c_str());
        return false;
    }

    QString animationsDir = "";
    cfg.getString(section_name, "AnimationsConfigDir", animationsDir);
    const std::string animations_dir = animationsDir.toStdString();

    QString modelShift = "";
    vsg::dvec3 shift(0.0, 0.0, 0.0);
    if (cfg.getString(section_name, "ModelShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
    }

    vsg::ref_ptr<vsg::MatrixTransform> vehicle_node = vsg::MatrixTransform::create();
    vehicle_node->matrix = vsg::translate(shift);
    vehicle_node->setValue("name", "vehicle");
    transform->addChild(vehicle_node);

    vsg::ref_ptr<AnimatedPagedLOD> vehicle_pagedLOD = AnimatedPagedLOD::create();
    vehicle_pagedLOD->animations_dir = animations_dir;
    vehicle_pagedLOD->filename = model_filename_path;
    vehicle_pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
    vehicle_pagedLOD->children[0] = vsg::PagedLOD::Child{0.1, {}};
    vehicle_pagedLOD->options = options;
    vehicle_node->addChild(vehicle_pagedLOD);
    animated_nodes.push_back(vehicle_pagedLOD);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleExterior::load_cabine_model(const std::string &cfg_path, CfgReader &cfg, vsg::ref_ptr<vsg::Options> options)
{
    const QString section_name = "Vehicle";
    QString modelName = "";

    cfg.getString(section_name, "CabineModel", modelName);
    if (modelName.isEmpty())
    {
        //LOG_WARN("Fail to read parameter <CabineModel> in config file: %s", cfg_path.c_str());
        return false;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string model_filename_path = fs.combinePath(fs.getVehicleModelsDir(), modelName.toStdString());
    if (!vsg::fileExists(model_filename_path))
    {
        LOG_WARN("Fail to find file: %s", model_filename_path.c_str());
        return false;
    }

    QString animationsDir = "";
    cfg.getString(section_name, "AnimationsConfigDir", animationsDir);
    const std::string animations_dir = animationsDir.toStdString();

    QString modelShift = "";
    vsg::dvec3 shift(0.0, 0.0, 0.0);
    if (cfg.getString(section_name, "CabineShift", modelShift))
    {
        std::istringstream ss(modelShift.toStdString());
        ss >> shift.x >> shift.y >> shift.z;
    }

    vsg::ref_ptr<vsg::MatrixTransform> cabine_node = vsg::MatrixTransform::create();
    cabine_node->matrix = vsg::translate(shift);
    cabine_node->setValue("name", "cabine");
    transform->addChild(cabine_node);

    vsg::ref_ptr<AnimatedPagedLOD> cabine_pagedLOD = AnimatedPagedLOD::create();
    cabine_pagedLOD->animations_dir = animations_dir;
    cabine_pagedLOD->filename = model_filename_path;
    cabine_pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
    cabine_pagedLOD->children[0] = vsg::PagedLOD::Child{0.1, {}};
    cabine_pagedLOD->options = options;
    cabine_node->addChild(cabine_pagedLOD);
    animated_nodes.push_back(cabine_pagedLOD);

    return true;
}

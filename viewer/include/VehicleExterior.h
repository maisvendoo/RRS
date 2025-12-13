#pragma once
#ifndef VEHICLE_EXTERIOR_H
#define VEHICLE_EXTERIOR_H

#include <vsg/core/Object.h>
#include <vsg/nodes/MatrixTransform.h>

class SoundManager;
//class AnimatedPagedLOD;       // Forward declare не работает,
#include "AnimatedPagedLOD.h"   // VehiclesHandler ругается на incomplete use

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehicleExterior final
{
public:
    VehicleExterior() = default;

    vsg::ref_ptr<vsg::MatrixTransform> transform = vsg::MatrixTransform::create();
    vsg::dvec3  position = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  orth = vsg::dvec3(0.0, 1.0, 0.0);
    vsg::dvec3  up = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dvec3  right = vsg::dvec3(1.0, 0.0, 0.0);
    vsg::dvec3  attitude = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  velocity = vsg::dvec3(0.0, 0.0, 0.0);
    std::vector<vsg::dvec3>  driver_pos = {vsg::dvec3(0.0, 0.0, 0.0)};
    std::vector<double>  driver_dir = {0};
    int         train_id = 0;
    int         orientation = 1;
    int         prev_vehicle = -1;
    int         next_vehicle = -1;

    std::vector<size_t> sounds_id = {};
    std::vector<vsg::ref_ptr<AnimatedPagedLOD>> animated_nodes;

    vsg::dvec3  saved_cabine_cam_shift = vsg::dvec3(0.0, 0.0, 0.0);
    double      saved_cabine_cam_right = 0.0;
    double      saved_cabine_cam_up = 0.0;
    double      saved_cabine_cam_fov = 64.0;

    /// Заданный индекс кабины
    size_t current_cabine_idx = 0;

    /// Фактический индекс кабины
    size_t controlled_cabine_idx = 0;


    void step(float t, float dt);
    void step(float t, float dt, std::vector<float> *server_signals);

    bool loadVehicle(const std::string& cfg_dir,
                     const std::string& cfg_file,
                     SoundManager *sm,
                     vsg::ref_ptr<vsg::Options> options);

private:

    /// Загрузка положения камеры в кабинах
    bool load_cabine_positions(const std::string& cfg_path, CfgReader& cfg);

    /// Загрузка звуков
    bool load_sounds(const std::string& cfg_path, CfgReader& cfg, SoundManager* sm);

    /// Загрузка моделей
    bool load_models(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);

    /// Методы под старый формат конфига с параметрами <ExtModelName> и <CabineModel>
    bool load_body_model(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
    bool load_cabine_model(const std::string& cfg_path, CfgReader& cfg, vsg::ref_ptr<vsg::Options> options);
};

#endif // VEHICLE_EXTERIOR_H

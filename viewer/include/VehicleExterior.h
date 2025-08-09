#pragma once
#ifndef VEHICLE_EXTERIOR_H
#define VEHICLE_EXTERIOR_H

#include "animations-list.h"

#include <vsg/core/Object.h>
#include <vsg/nodes/MatrixTransform.h>

class SoundManager;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehicleExterior final
{
public:
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

    vsg::ref_ptr<animations_t> animations = animations_t::create();
    std::vector<size_t> sounds_id = {};

    vsg::dvec3  saved_cabine_cam_shift = vsg::dvec3(0.0, 0.0, 0.0);
    double      saved_cabine_cam_right = 0.0;
    double      saved_cabine_cam_up = 0.0;
    double      saved_cabine_cam_fov = 64.0;

    /// Заданный индекс кабины
    size_t current_cabine_idx = 0;

    /// Фактический индекс кабины
    size_t controlled_cabine_idx = 0;

    VehicleExterior() = default;

    void step(float t, float dt, const vsg::dvec3* camera_pos);

    bool loadVehicle(const std::string& cfg_dir,
                     const std::string& cfg_file,
                     SoundManager *sm,
                     vsg::ref_ptr<vsg::Viewer> viewer,
                     vsg::ref_ptr<vsg::Options> options);

private:
    void load_sounds(const std::string& sounds_dir, SoundManager *sm);
};

#endif // VEHICLE_EXTERIOR_H

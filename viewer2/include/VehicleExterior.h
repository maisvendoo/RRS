#ifndef VEHICLE_EXTERIOR_H
#define VEHICLE_EXTERIOR_H

#include "animations-list.h"
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/maths/vec3.h>

class SoundManager;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehicleExterior
{
public:

    vsg::ref_ptr<vsg::MatrixTransform> transform = vsg::MatrixTransform::create();
    vsg::dvec3  position = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  orth = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  up = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dvec3  right = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  attitude = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  driver_pos = vsg::dvec3(0.0, 0.0, 0.0);
    int         train_id = 0;
    int         orientation = 1;
    int         prev_vehicle = -1;
    int         next_vehicle = -1;

    animations_t animations = {};
    //displays_t   *displays = new displays_t();
    std::vector<size_t> sounds_id = {};

    VehicleExterior(){};

    void step(float t, float dt);

    bool loadVehicle(std::string& cfg_dir, std::string& cfg_file, SoundManager *sm, vsg::ref_ptr<vsg::Options> options);

private:

    vsg::ref_ptr<vsg::MatrixTransform> loadModel(const std::string &modelName, const std::string &textureName, vsg::ref_ptr<vsg::Options> options);

    void load_animations(const std::string& animations_dir);

    void load_model_animations(const std::string& animations_dir);

    void load_sounds(const std::string& sounds_dir, SoundManager *sm);

    void load_displays(const std::string& cfg_path);
};

#endif // VEHICLE_EXTERIOR_H

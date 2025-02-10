#ifndef ANALOG_ROTATION_H
#define ANALOG_ROTATION_H

#include "ConfigReader.h"
#include "ProcAnimation.h"
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/nodes/MatrixTransform.h>

class AnalogRotation : public ProcAnimation
{
public:
    AnalogRotation(vsg::MatrixTransform* transform);

private:
    float min_angle = 0.0f;
    float max_angle = vsg::PIf;
    float angle = 0.0f;

    float cur_pos = 0.0f;

    bool infinity = false;

    vsg::dvec3 axis = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dmat4 matrix;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader& cfg);

    void update();
};

#endif // ANALOG_ROTATION_H

#ifndef ANALOG_ROTATION_H
#define ANALOG_ROTATION_H

#include "ProcAnimation.h"

#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

class CfgReader;

namespace vsg
{
    class MatrixTransform;
}

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

    void anim_step(float t, float dt) override;

    bool load_config(CfgReader &cfg) override;

    void update();
};

#endif // ANALOG_ROTATION_H

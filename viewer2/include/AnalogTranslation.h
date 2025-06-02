#ifndef ANALOG_TRANSLATION_H
#define ANALOG_TRANSLATION_H

#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

class CfgReader;

namespace vsg
{
    class MatrixTransform;
}

class AnalogTranslation : public ProcAnimation
{
public:
    AnalogTranslation(vsg::MatrixTransform* transform);

    void update();

private:
    float motion = 0.0f;

    float cur_pos = 0.0f;

    vsg::dvec3 axis = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dmat4 matrix;

    void anim_step(float t, float dt) override;

    bool load_config(CfgReader& cfg) override;
};

#endif // ANALOG_TRANSLATION_H

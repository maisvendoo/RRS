#ifndef ANALOG_TRANSLATION_H
#define ANALOG_TRANSLATION_H

#include "ConfigReader.h"
#include "ProcAnimation.h"
#include <vsg/nodes/MatrixTransform.h>

class AnalogTranslation : public ProcAnimation
{
public:
    AnalogTranslation(vsg::MatrixTransform* transform);

private:
    float min_motion = 0.0f;
    float max_motion = 0.0f;
    float motion = 0.0f;

    float cur_pos = 0.0f;

    vsg::dvec3 axis = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dmat4 matrix;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader& cfg);

    void update();
};

#endif // ANALOG_TRANSLATION_H

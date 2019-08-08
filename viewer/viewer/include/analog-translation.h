#ifndef     ANALOG_TRANSLATION_H
#define     ANALOG_TRANSLATION_H

#include    "proc-animation.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnalogTranslation : public ProcAnimation
{
public:

    AnalogTranslation(osg::MatrixTransform *transform);

    ~AnalogTranslation();

private:

    float       min_motion;
    float       max_motion;
    float       motion;

    float       cur_pos;

    osg::Vec3   axis;
    osg::Matrix matrix;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader &cfg);

    void update();
};

#endif // ANALOG_TRANSLATION_H

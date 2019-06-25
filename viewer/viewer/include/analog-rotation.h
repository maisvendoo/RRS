#ifndef     ANALOG_ROTATION_H
#define     ANALOG_ROTATION_H

#include    "proc-animation.h"

class AnalogRotation : public ProcAnimation
{
public:

    AnalogRotation(osg::MatrixTransform *transform);

    ~AnalogRotation();

private:

    float       min_angle;
    float       max_angle;
    float       angle;

    float       cur_pos;

    osg::Vec3   axis;
    osg::Matrix matrix;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader &cfg);

    void update();
};

#endif // ANALOG_ROTATION_H

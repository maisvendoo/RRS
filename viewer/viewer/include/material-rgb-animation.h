#ifndef     MATERIAL_RGB_ANIMATION_H
#define     MATERIAL_RGB_ANIMATION_H

#include    "proc-animation.h"

#include    <osg/Material>
#include    <osg/StateSet>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class   MaterialRGBAnimation : public ProcAnimation
{
public:

    MaterialRGBAnimation(osg::Material *mat, osg::Drawable *drawable);

    ~MaterialRGBAnimation();

private:

    osg::Material   *mat;
    osg::Drawable   *drawable;
    osg::ref_ptr<osg::StateSet>   stateset;

    float           cur_pos_r;
    float           cur_pos_b;
    float           cur_pos_g;

    float           pos_r;
    float           pos_b;
    float           pos_g;

    osg::Vec4       color;
    osg::Vec4       emission_color;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader &cfg);

    float getChannelState(float pos, unsigned char channel);

    void update();
};

#endif // MATERIAL_RGB_ANIMATION_H

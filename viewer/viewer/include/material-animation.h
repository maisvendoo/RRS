#ifndef     MATERIAL_ANIMATION_H
#define     MATERIAL_ANIMATION_H

#include    "proc-animation.h"

#include    <osg/Material>
#include    <osg/StateSet>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MaterialAnimation : public ProcAnimation
{
public:

    MaterialAnimation(osg::Material *mat, osg::Drawable *drawable);

    ~MaterialAnimation();

private:


    osg::Material   *mat;
    osg::Drawable   *drawable;
    osg::ref_ptr<osg::StateSet>   stateset;    

    float           cur_pos;

    osg::Vec4       color;
    osg::Vec4       emission_color;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader &cfg);

    void update();
};

#endif // MATERIAL_ANIMATION_H

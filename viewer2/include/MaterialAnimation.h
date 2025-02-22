#ifndef MATERIAL_ANIMATION_H
#define MATERIAL_ANIMATION_H

#include "ProcAnimation.h"
#include <vsg/maths/vec4.h>
#include <vsg/state/material.h>

class CfgReader;

class MaterialAnimation : public ProcAnimation
{
public:
    explicit MaterialAnimation(vsg::PbrMaterial& material);

private:
    vsg::PbrMaterial& material;

    float cur_pos = 0.0f;

    vsg::vec4 color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vsg::vec4 emission_color = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    void anim_step(float t, float dt);

    bool load_config(CfgReader& cfg);

    void update();
};

#endif // MATERIAL_ANIMATION_H

#ifndef MATERIAL_ANIMATION_H
#define MATERIAL_ANIMATION_H

#include "ConfigReader.h"
#include "ProcAnimation.h"
#include <vsg/maths/vec4.h>
#include <vsg/state/material.h>

class MaterialAnimation : public ProcAnimation
{
public:
    explicit MaterialAnimation(vsg::PbrMaterial& material);

private:
    vsg::PbrMaterial& material;

    float cur_pos = 0.0f;

    vsg::vec4 color;
    vsg::vec4 emission_color;

    void anim_step(float t, float dt);

    bool load_config(ConfigReader& cfg);

    void update();
};

#endif // MATERIAL_ANIMATION_H

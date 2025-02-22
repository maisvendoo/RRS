#ifndef MATERIAL_RGB_ANIMATION_H
#define MATERIAL_RGB_ANIMATION_H

#include "ProcAnimation.h"
#include <vsg/state/material.h>

class CfgReader;

class MaterialRgbAnimation : public ProcAnimation
{
public:
    MaterialRgbAnimation(vsg::PbrMaterial& material);

private:
    vsg::PbrMaterial& material;

    float cur_pos_r = 0.0f;
    float cur_pos_g = 0.0f;
    float cur_pos_b = 0.0f;

    float pos_r = 0.0f;
    float pos_g = 0.0f;
    float pos_b = 0.0f;

    vsg::vec4 color;
    vsg::vec4 emission_color;

    void anim_step(float t, float dt) override;

    bool load_config(CfgReader& cfg) override;

    float getChannelState(float pos, unsigned char channel);

    void update();
};

#endif // MATERIAL_RGB_ANIMATION_H

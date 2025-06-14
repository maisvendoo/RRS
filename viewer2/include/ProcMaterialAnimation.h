#ifndef MATERIAL_ANIMATION_H
#define MATERIAL_ANIMATION_H

#include "ProcAnimation.h"

#include <vsg/maths/vec4.h>
#include <vsg/core/Value.h>
#include <vsg/state/material.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcMaterialAnimation : public vsg::Inherit<ProcAnimation, ProcMaterialAnimation>
{
public:
    explicit ProcMaterialAnimation(vsg::ref_ptr<vsg::PbrMaterialValue> data);

private:
    float intensity = 0.0f;

    vsg::ref_ptr<vsg::PbrMaterialValue> material_value;

    vsg::vec4 color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vsg::vec4 emission_color = vsg::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;
};

#endif // MATERIAL_ANIMATION_H

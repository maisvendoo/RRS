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
    explicit ProcMaterialAnimation(vsg::ref_ptr<vsg::PbrMaterialValue> in_material_data);

private:
    vsg::ref_ptr<vsg::PbrMaterialValue> material_value;

    vsg::vec4 base_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vsg::vec4 emission_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;
};

#endif // MATERIAL_ANIMATION_H

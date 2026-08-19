#ifndef DISPLAY_ANIMATION_H
#define DISPLAY_ANIMATION_H

#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec4.h>
#include <vsg/state/material.h>

#include <QImage>

#include <cstddef>
#include <vector>

class AbstractDisplay;
class CfgReader;

namespace vsg
{

class Image;

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcDisplayAnimation final : public vsg::Inherit<ProcAnimation, ProcDisplayAnimation>
{
public:
    ProcDisplayAnimation(
        vsg::ref_ptr<vsg::Image> in_image_color,
        vsg::ref_ptr<vsg::Image> in_image_emissive,
        vsg::ref_ptr<vsg::PbrMaterialValue> in_material_value
    );

    std::size_t getSignalID() const override;

private:
    void anim_step(float t, float dt) override;

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;

private:
    vsg::ref_ptr<vsg::Image> image_color;
    vsg::ref_ptr<vsg::Image> image_emissive;

    vsg::ref_ptr<vsg::PbrMaterialValue> material_value;

    vsg::vec4 base_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vsg::vec4 emission_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    //------------------------------------------------------------------------------

    std::vector<float>* prev_signals = nullptr;

    AbstractDisplay* display = nullptr;
    QImage qimage;
    bool is_color_repaint = true;
    bool is_emissive_repaint = true;
};

#endif // DISPLAY_ANIMATION_H

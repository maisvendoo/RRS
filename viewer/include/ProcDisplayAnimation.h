#ifndef DISPLAY_ANIMATION_H
#define DISPLAY_ANIMATION_H

#include "ProcAnimation.h"
#include "display.h"

#include <vsg/maths/vec4.h>
#include <vsg/core/Value.h>
#include <vsg/state/material.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcDisplayAnimation : public vsg::Inherit<ProcAnimation, ProcDisplayAnimation>
{
public:
    explicit ProcDisplayAnimation(vsg::ref_ptr<vsg::Image> in_image_data,
                                  vsg::ref_ptr<vsg::PbrMaterialValue> in_material_data);

    std::size_t getSignalID() const override;

private:
    vsg::ref_ptr<vsg::Image> image_data;
    bool prev_sin_t_positive = true;

    std::string module_dir = "";
    std::string module_name = "";
    std::string module_path = "";

    vsg::ref_ptr<vsg::PbrMaterialValue> material_value;

    vsg::vec4 base_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vsg::vec4 emission_color = vsg::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    void anim_step(float t, float dt) override;

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;

    //------------------------------------------------------------------------------

    std::vector<float>* prev_signals = nullptr;

    AbstractDisplay* display = nullptr;
    QImage qimage;
};

#endif // DISPLAY_ANIMATION_H

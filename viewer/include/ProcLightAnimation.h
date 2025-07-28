#ifndef     LIGHT_ANIMATION_H
#define     LIGHT_ANIMATION_H

#include    <ProcAnimation.h>

#include    <vsg/maths/vec4.h>
#include    <vsg/core/Value.h>
#include    <vsg/lighting/Light.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcLightAnimation final : public vsg::Inherit<ProcAnimation, ProcLightAnimation>
{
public:

    explicit ProcLightAnimation(vsg::ref_ptr<vsg::Light> in_light);

    void setLight(vsg::ref_ptr<vsg::Light> light)
    {
        this->light = light;

        if (!is_fixed_signal)
        {
            cur_signal = 0.0f;
        }

        update(cur_signal);
    }

private:

    vsg::ref_ptr<vsg::Light> light;

    vsg::vec3 light_color = vsg::vec3(1.0, 1.0, 1.0);

    double max_intensity = 1.0;

    void update(float current_signal) override;

    bool load_config(CfgReader &cfg) override;

    void load_common_settings(CfgReader &cfg);

    void load_spotlight_settings(vsg::SpotLight *sl, CfgReader &cfg);
 };

#endif // LIGHT_ANIMATION_H

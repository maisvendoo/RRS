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

    std::int32_t signal_id2 = -1;
    std::int32_t signal_id3 = -1;
    float cur_signal2 = 0.0f;
    float cur_signal3 = 0.0f;

    vsg::vec3 light_color = vsg::vec3(1.0f, 1.0f, 1.0f);
    vsg::vec3 light_color2 = vsg::vec3(1.0f, 1.0f, 1.0f);
    vsg::vec3 light_color3 = vsg::vec3(1.0f, 1.0f, 1.0f);

    double max_intensity = 1.0;

    void anim_step(float t, float dt) override;

    void update(float current_signal) override;

    bool load_config(CfgReader &cfg) override;

    void load_common_settings(CfgReader &cfg);

    void load_spotlight_settings(vsg::SpotLight *sl, CfgReader &cfg);

    void load_pointlight_settings(vsg::PointLight *pl, CfgReader &cfg);
 };

#endif // LIGHT_ANIMATION_H

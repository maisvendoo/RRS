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

private:

    vsg::ref_ptr<vsg::Light> light;

    void update(float current_signal) override;

    bool load_config(CfgReader &cfg) override;
 };

#endif // LIGHT_ANIMATION_H

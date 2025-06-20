#ifndef ANALOG_TRANSLATION_H
#define ANALOG_TRANSLATION_H

#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

class CfgReader;

namespace vsg
{
    class MatrixTransform;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcTranslationAnimation final : public vsg::Inherit<ProcAnimation, ProcTranslationAnimation>
{
public:
    explicit ProcTranslationAnimation(vsg::ref_ptr<vsg::MatrixTransform> transform);

    void setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform);

private:
    float motion = 0.0f;

    vsg::dvec3 axis = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dmat4 matrix;

    vsg::ref_ptr<vsg::MatrixTransform> transform_node = nullptr;

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;
};

#endif // ANALOG_TRANSLATION_H

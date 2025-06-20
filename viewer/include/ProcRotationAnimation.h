#ifndef ANALOG_ROTATION_H
#define ANALOG_ROTATION_H

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
class ProcRotationAnimation final : public vsg::Inherit<ProcAnimation, ProcRotationAnimation>
{
public:
    explicit ProcRotationAnimation(vsg::ref_ptr<vsg::MatrixTransform> transform);

    void setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform);

private:
    float angle = 0.0f;
    bool infinity = false;

    vsg::dvec3 axis = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dmat4 matrix;

    vsg::ref_ptr<vsg::MatrixTransform> transform_node = nullptr;

    void update(float current_signal) override;

    bool load_config(CfgReader &cfg) override;
};

#endif // ANALOG_ROTATION_H

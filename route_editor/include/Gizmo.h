#ifndef GIZMO_H
#define GIZMO_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

struct settings_t;

namespace vsg
{

class Node;

}

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(const settings_t& settings);

private:
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
};

#endif // GIZMO_H

#ifndef GIZMO_H
#define GIZMO_H

#include "SelectedObjectsMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>

struct settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class LineSegmentIntersector;
class LookAt;
class MoveEvent;
class Node;

}

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(
        const settings_t& settings,
        vsg::ref_ptr<vsg::LookAt> look_at,
        const SelectedObjectsMap& selected_objects
    );

    bool handle_intersections(
        vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
    );

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    void update_position();
    void update_scale();

private:
    vsg::ref_ptr<vsg::Node> create_arrow(
        const vsg::vec3& direction,
        const vsg::vec3& color
    );

    vsg::ref_ptr<vsg::Node> create_plane(
        const float plane_size,
        const int zero_component_index
    );

    vsg::ref_ptr<vsg::Node> create_line(
        const float plane_size,
        const float line_size,
        const int plane_component_index,
        const vsg::vec3& color
    );

private:
    const settings_t& settings;
    vsg::ref_ptr<vsg::LookAt> look_at;
    const SelectedObjectsMap& selected_objects;
    vsg::vec3 position;
    vsg::Builder builder;
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
    vsg::ref_ptr<vsg::Node> plane_yz;
    vsg::ref_ptr<vsg::Node> plane_xz;
    vsg::ref_ptr<vsg::Node> plane_xy;
    vsg::Mask* plane_mask_yz = nullptr;
    vsg::Mask* plane_mask_xz = nullptr;
    vsg::Mask* plane_mask_xy = nullptr;
};

#endif // GIZMO_H

#ifndef EDITOR_GIZMO_H
#define EDITOR_GIZMO_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Switch.h>
#include <vsg/utils/Builder.h>

class Camera;
struct EditorContext;
struct gizmo_settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class MatrixTransform;
class MoveEvent;
class Node;

} // namespace vsg

class Gizmo : public vsg::Inherit<vsg::Switch, Gizmo>
{
public:
    Gizmo(EditorContext& editor_ctx);

    bool handle_intersections();

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    const vsg::dvec3& get_curr_pos() const;

    void update_visibility();
    void update_position();

private:
    EditorContext& editor_ctx;
    const Camera& camera;
    const gizmo_settings_t& gizmo_settings;

    vsg::Builder builder;
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform;
    vsg::ref_ptr<vsg::Node> arrows[3];
    vsg::ref_ptr<vsg::Switch> planes_switch;
    vsg::ref_ptr<vsg::Switch> lines_switch;

    vsg::dvec3 curr_pos;
    vsg::dvec3 prev_intersect_pos;
    vsg::dvec3 total_translation;
    double scale;

    int active_arrow_index = -1;
    int active_plain_index = -1;
};

#endif // EDITOR_GIZMO_H

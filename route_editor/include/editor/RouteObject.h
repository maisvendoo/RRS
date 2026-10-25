#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include "editor/RouteObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

struct EditorContext;
class SingleSwitch;

namespace vsg
{

class PagedLOD;

}

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(EditorContext& editor_ctx,
                const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
                const std::string& label, const vsg::dvec3& position,
                const vsg::dvec3& rot_deg = {0.0, 0.0, 0.0},
                const vsg::dvec3& scale = {1.0, 1.0, 1.0});

    const vsg::dvec3& get_position() const;
    const vsg::dvec3& get_rot_deg() const;
    const vsg::dvec3& get_scale() const;

    const vsg::dmat4& get_initial_matrix() const;
    const vsg::dbox& get_bounds() const;

    bool get_is_selected() const;
    bool get_is_hidden() const;

    void set_position(const vsg::dvec3& position);
    void set_rot_deg(const vsg::dvec3& rot_deg);
    void set_scale(const vsg::dvec3& scale);

    void move(const vsg::dvec3& delta_position);

    void rotate_around_pivot(const vsg::dvec3& pivot, const vsg::dvec3& axis,
                             double radians, const vsg::dmat4& matrix);

    void scale_relative_to_pivot(const vsg::dvec3& pivot,
                                 const vsg::dvec3& scale,
                                 const vsg::dmat4& matrix);

    void hide();
    RouteObjectsIterator show();

    bool select();
    RouteObjectsIterator deselect();

    vsg::ref_ptr<RouteObject> copy() const;

    void save_matrix();
    void set_matrix(const vsg::dmat4& matrix);

public:
    std::string label;

private:
    void update_matrix();
    void update_bounds();
    void decompose_matrix();

private:
    EditorContext& editor_ctx;

    vsg::dvec3 position;
    vsg::dvec3 rot_deg;
    vsg::dvec3 scale;

    vsg::dmat4 initial_matrix;
    vsg::dbox bounds;

    bool is_selected = false;
    bool is_hidden = false;

    vsg::ref_ptr<SingleSwitch> paged_lod_switch;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
    vsg::ref_ptr<SingleSwitch> outline_switch;
};

#endif // ROUTE_OBJECT_H

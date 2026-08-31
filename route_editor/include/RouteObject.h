#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include "RouteObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

struct EditorContext;
class Gizmo;
class SingleSwitch;

namespace vsg
{

class PagedLOD;

}

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(
        EditorContext& context,
        const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
        const vsg::ref_ptr<Gizmo>& gizmo,
        const std::string& label,
        const vsg::dvec3& translation,
        const vsg::dvec3& rotation_deg = {0.0, 0.0, 0.0},
        const vsg::dvec3& scale = {1.0, 1.0, 1.0}
    );

    const vsg::dvec3& get_translation() const;
    const vsg::dvec3& get_rotation_deg() const;
    const vsg::dvec3& get_scale() const;

    const vsg::dmat4& get_initial_matrix() const;
    const vsg::dbox& get_bounds() const;

    bool get_is_selected() const;
    bool get_is_hidden() const;

    void set_translation(const vsg::dvec3& translation);
    void set_rotation_deg(const vsg::dvec3& rotation_deg);
    void set_scale(const vsg::dvec3& scale);

    void move(const vsg::dvec3& translation);

    void rotate_around_pivot(const vsg::dvec3& pivot, const vsg::dvec3& axis,
        double radians, const vsg::dmat4& matrix);

    void scale_relative_to_pivot(const vsg::dvec3& pivot, const vsg::dvec3& scale,
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
    EditorContext& context_;

    vsg::dvec3 translation_;
    vsg::dvec3 rotation_deg_;
    vsg::dvec3 scale_;

    vsg::dmat4 initial_matrix_;
    vsg::dbox bounds_;

    bool is_selected_ = false;
    bool is_hidden_ = false;

    vsg::ref_ptr<SingleSwitch> paged_lod_switch_;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod_;
    const vsg::ref_ptr<Gizmo>& gizmo;
    vsg::ref_ptr<SingleSwitch> outline_switch_;
};

#endif // ROUTE_OBJECT_H

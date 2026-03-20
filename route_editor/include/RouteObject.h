#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include "RouteObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

struct EditorContext;
class Gizmo;
class RouteObject;
class SingleSwitch;

namespace vsg
{

class PagedLOD;
class Viewer;

}

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(EditorContext& context, vsg::ref_ptr<vsg::PagedLOD> paged_lod,
        const std::string& label, vsg::vec3 translation, vsg::vec3 rotation_deg);

    vsg::vec3 get_translation() const;
    vsg::vec3 get_rotation_deg() const;
    vsg::vec3 get_scale() const;

    const vsg::dmat4& get_initial_matrix() const;

    const vsg::box& get_bounds() const;

    bool get_is_selected() const;
    bool get_is_hidden() const;

    void set_translation(vsg::vec3 translation);
    void set_rotation_deg(vsg::vec3 rotation_deg, bool update_matrix);
    void set_scale(vsg::vec3 scale, bool update_matrix);

    void move(vsg::vec3 translation);
    void rotate(vsg::vec3 rotation_deg, bool update_matrix);
    void scale(vsg::vec3 scale, bool update_matrix);

    void rotate_around_pivot(vsg::vec3 pivot, vsg::vec3 axis, float radians,
        const vsg::dmat4& matrix);

    void scale_relative_to_pivot(vsg::vec3 pivot, vsg::vec3 scale,
        const vsg::dmat4& matrix);

    void hide();
    RouteObjectsIterator show();

    void select();
    RouteObjectsIterator deselect();

    void save_matrix();

    void set_matrix(vsg::dmat4 matrix);

    void update_matrix();
    void update_bounds();

public:
    std::string label;

private:
    EditorContext& context;

    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
    vsg::vec3 scale_value = {1.0f, 1.0f, 1.0f};

    vsg::dmat4 initial_matrix;

    vsg::box bounds;

    bool is_selected = false;
    bool is_hidden = false;

    vsg::ref_ptr<SingleSwitch> paged_lod_switch;
    vsg::ref_ptr<SingleSwitch> outline_switch;
};

#endif // ROUTE_OBJECT_H

#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <list>
#include <string>

struct EditorContext;
class RouteObject;
class SingleSwitch;

namespace vsg
{

class PagedLOD;

}

using RouteObjects = std::list<vsg::ref_ptr<RouteObject>>;
using RouteObjectsIterator = RouteObjects::iterator;

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(
        EditorContext& context,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod,
        const std::string& label,
        vsg::vec3 translation,
        vsg::vec3 rotation_deg,
        vsg::vec3 scale = {1.0f, 1.0f, 1.0f}
    );

    const vsg::dvec3& get_translation() const;
    const vsg::dvec3& get_rotation_deg() const;
    const vsg::dvec3& get_scale() const;

    const vsg::dmat4& get_initial_matrix() const;
    const vsg::dbox& get_bounds() const;

    bool get_is_selected() const;
    bool get_is_hidden() const;

    void set_translation(const vsg::dvec3& translation);
    void set_rotation_deg(vsg::vec3 rotation_deg);
    void set_scale(vsg::vec3 scale);

    void move(vsg::vec3 translation);

    void rotate_around_pivot(vsg::vec3 pivot, vsg::vec3 axis, float radians,
        const vsg::dmat4& matrix);

    void scale_relative_to_pivot(vsg::vec3 pivot, vsg::vec3 scale,
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

private:
    static EditorContext* s_context;

    vsg::dvec3 translation;
    vsg::dvec3 rotation_deg;
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

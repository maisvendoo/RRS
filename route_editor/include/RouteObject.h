#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <list>
#include <string>

class RouteObject;
class SingleSwitch;

namespace vsg
{

class PagedLOD;
class Viewer;

}

using RouteObjects = std::list<vsg::ref_ptr<RouteObject>>;
using RouteObjectsIterator = RouteObjects::iterator;

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    RouteObject(vsg::ref_ptr<vsg::PagedLOD> paged_lod, const std::string& label,
        vsg::vec3 translation, vsg::vec3 rotation_deg);

    vsg::vec3 get_translation() const;
    vsg::vec3 get_rotation_deg() const;
    vsg::vec3 get_scale() const;

    vsg::vec3 get_initial_translation() const;
    vsg::vec3 get_initial_rotation_deg() const;
    vsg::vec3 get_initial_scale() const;

    const vsg::box& get_bounds() const;

    static void set_observer_viewer(
        vsg::observer_ptr<vsg::Viewer> observer_viewer);

    static void set_selected_objects(RouteObjects* selected_objects);
    static void set_hidden_objects(RouteObjects* route_objects);

    void set_translation(vsg::vec3 translation, bool update_matrix);
    void set_rotation_deg(vsg::vec3 rotation_deg, bool update_matrix);
    void set_scale(vsg::vec3 scale, bool update_matrix);

    void move(vsg::vec3 translation, bool update_matrix);
    void rotate(vsg::vec3 rotation_deg, bool update_matrix);
    void scale(vsg::vec3 scale, bool update_matrix);

    void rotate_relative_to_point(vsg::vec3 point, vsg::vec3 rotation_deg,
        bool update_matrix);

    void scale_relative_to_point(vsg::vec3 point, vsg::vec3 scale,
        bool update_matrix);

    void hide();
    void show() const;

    void select();
    void deselect() const;

    void save_translation();
    void save_rotation();
    void save_scale();

    void update_matrix();
    void update_bounds();

public:
    std::string label;

private:
    static vsg::observer_ptr<vsg::Viewer> s_observer_viewer;
    static RouteObjects* s_selected_objects;
    static RouteObjects* s_hidden_objects;

    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
    vsg::vec3 scale_value = {1.0f, 1.0f, 1.0f};

    vsg::vec3 initial_translation;
    vsg::vec3 initial_rotation_deg;
    vsg::vec3 initial_scale;

    vsg::box bounds;

    vsg::ref_ptr<SingleSwitch> paged_lod_switch;
    vsg::ref_ptr<SingleSwitch> outline_switch;
};

#endif // ROUTE_OBJECT_H

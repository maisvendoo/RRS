#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

class Outline;
class SwitchGroup;

namespace vsg
{

class PagedLOD;

}

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

    void set_translation(vsg::vec3 translation, bool update_matrix = true);
    void set_rotation_deg(vsg::vec3 rotation_deg, bool update_matrix = true);
    void set_scale(vsg::vec3 scale, bool update_matrix = true);

    void move(vsg::vec3 translation, bool update_matrix = true);

    void save_translation();
    void save_rotation();
    void save_scale();

    void update_matrix();
    void update_bounds();

    vsg::ref_ptr<SwitchGroup> get_switch_group() const;
    vsg::ref_ptr<vsg::PagedLOD> get_paged_lod() const;

public:
    enum class State
    {
        INITIAL,
        MOVING,
        ROTATING,
        SCALING
    };

    State state = State::INITIAL;
    std::string label;

    vsg::ref_ptr<Outline> outline;

private:
    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
    vsg::vec3 scale = {1.0f, 1.0f, 1.0f};

    vsg::vec3 initial_translation;
    vsg::vec3 initial_rotation_deg;
    vsg::vec3 initial_scale;

    vsg::box bounds;

    vsg::ref_ptr<SwitchGroup> switch_group;
    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
};

#endif // ROUTE_OBJECT_H

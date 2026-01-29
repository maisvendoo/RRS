#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include <vsg/core/Inherit.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <string>

class RouteObject : public vsg::Inherit<vsg::MatrixTransform, RouteObject>
{
public:
    enum class State
    {
        INITIAL,
        MOVING,
        ROTATING
    };

    vsg::vec3 get_translation() const;
    vsg::vec3 get_rotation_deg() const;
    vsg::vec3 get_scale() const;

    void set_translation(vsg::vec3 translation, bool update_matrix = true);
    void set_rotation_deg(vsg::vec3 rotation_deg, bool update_matrix = true);
    void set_scale(vsg::vec3 scale, bool update_matrix = true);

public:
    State state = State::INITIAL;
    std::string name;

private:
    void update_matrix();

private:
    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
    vsg::vec3 scale;
};

#endif // ROUTE_OBJECT_H

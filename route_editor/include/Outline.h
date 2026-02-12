#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Group.h>

class RouteObject;
struct settings_t;

class Outline : public vsg::Inherit<vsg::Group, Outline>
{
public:
    Outline(const settings_t& settings, const RouteObject* object);
};

#endif // OUTLINE_H

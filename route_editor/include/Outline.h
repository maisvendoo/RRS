#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

class RouteObject;
struct settings_t;

class Outline : public vsg::Inherit<vsg::Group, Outline>
{
public:
    Outline(const settings_t& settings, vsg::ref_ptr<RouteObject> object);
};

#endif // OUTLINE_H

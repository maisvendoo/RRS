#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

class RouteObject;
struct settings_t;

namespace vsg
{

class PagedLOD;

}

class Outline : public vsg::Inherit<vsg::Group, Outline>
{
public:
    Outline(const settings_t& settings, vsg::ref_ptr<vsg::PagedLOD> paged_lod);
};

#endif // OUTLINE_H

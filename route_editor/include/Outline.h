#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

struct settings_t;

namespace vsg
{

class PagedLOD;
class Viewer;

}

class Outline : public vsg::Inherit<vsg::Group, Outline>
{
public:
    Outline(
        const settings_t& settings,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod,
        vsg::observer_ptr<vsg::Viewer> observer_viewer
    );
};

#endif // OUTLINE_H

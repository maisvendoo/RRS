#ifndef OUTLINE_NEW_H
#define OUTLINE_NEW_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/StateGroup.h>

struct settings_t;

namespace vsg
{

class PagedLOD;

}

class OutlineNew : public vsg::Inherit<vsg::StateGroup, OutlineNew>
{
public:
    OutlineNew(const settings_t& settings,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod);
};

#endif // OUTLINE_NEW_H

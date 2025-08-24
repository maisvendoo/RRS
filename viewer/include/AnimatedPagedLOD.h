#ifndef ANIMATED_PAGED_LOD_H
#define ANIMATED_PAGED_LOD_H

#include "animations-list.h"

#include <vsg/nodes/PagedLOD.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimatedPagedLOD : public vsg::Inherit<vsg::PagedLOD, AnimatedPagedLOD>
{
public:
    AnimatedPagedLOD() : Inherit() {};

    AnimatedPagedLOD(const AnimatedPagedLOD& rhs, const vsg::CopyOp& copyop) :
        Inherit(rhs, copyop)
    {
        filename = rhs.filename;
        bound = rhs.bound;

        children[0].minimumScreenHeightRatio = rhs.children[0].minimumScreenHeightRatio;
        children[0].node = copyop(rhs.children[0].node);
        children[1].minimumScreenHeightRatio = rhs.children[1].minimumScreenHeightRatio;
        children[1].node = copyop(rhs.children[1].node);
    }

    std::string animations_dir;

    vsg::ref_ptr<animations_t> animations_map = animations_t::create();

protected:
    ~AnimatedPagedLOD() = default;
};

#endif // ANIMATED_PAGED_LOD_H

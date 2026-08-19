#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/utils/Builder.h>

struct EditorContext;

namespace vsg
{

class Node;
class Options;
class PagedLOD;

}

class OutlineBuilder : public vsg::Inherit<vsg::Object, OutlineBuilder>
{
public:
    OutlineBuilder();

    vsg::ref_ptr<vsg::Node> create_outline(
        vsg::ref_ptr<vsg::PagedLOD> paged_lod);

private:
    vsg::ref_ptr<vsg::Options> options_;
    vsg::Builder builder_;
};

#endif // OUTLINE_H

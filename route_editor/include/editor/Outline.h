#ifndef EDITOR_OUTLINE_H
#define EDITOR_OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/utils/Builder.h>

struct EditorContext;

namespace vsg
{

class Node;
class PagedLOD;

} // namespace vsg

class OutlineBuilder : public vsg::Inherit<vsg::Object, OutlineBuilder>
{
public:
    OutlineBuilder();

    vsg::ref_ptr<vsg::Node> create_outline(
        const vsg::ref_ptr<vsg::PagedLOD>& paged_lod);

private:
    vsg::Builder builder;
};

#endif // EDITOR_OUTLINE_H

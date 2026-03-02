#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>
#include <vsg/utils/Builder.h>

struct EditorContext;
struct settings_t;

namespace vsg
{

class Node;
class PagedLOD;
class Viewer;

}

class OutlineBuilder : public vsg::Inherit<vsg::Object, OutlineBuilder>
{
public:
    OutlineBuilder(const EditorContext& context);

    vsg::ref_ptr<vsg::Node> create_outline(vsg::ref_ptr<vsg::PagedLOD> paged_lod);

private:
    const EditorContext& context;

    vsg::ref_ptr<vsg::Options> options;
    vsg::Builder builder;
};

#endif // OUTLINE_H

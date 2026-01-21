#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>

namespace vsg
{

class Options;
class PagedLOD;
class Viewer;

}

class Outline : public vsg::Inherit<vsg::MatrixTransform, Outline>
{
public:
    Outline(vsg::observer_ptr<vsg::Viewer> observer_viewer);

    void update(vsg::ref_ptr<vsg::PagedLOD> paged_lod);

private:
    vsg::observer_ptr<vsg::Viewer> observer_viewer;
    vsg::ref_ptr<vsg::Options> options;
    vsg::Builder builder;
};

#endif // OUTLINE_H

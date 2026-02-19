#ifndef OUTLINE_H
#define OUTLINE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

struct settings_t;

namespace vsg
{

class Node;
class PagedLOD;
class Viewer;

}

class Outline : public vsg::Inherit<vsg::Group, Outline>
{
public:
    Outline(vsg::ref_ptr<vsg::PagedLOD> paged_lod);

    void load(vsg::observer_ptr<vsg::Viewer> observer_viewer);

    static void set_settings(const settings_t* settings);

private:
    static const settings_t* s_settings;

    vsg::ref_ptr<vsg::PagedLOD> paged_lod;
    vsg::ref_ptr<vsg::Node> box;
};

#endif // OUTLINE_H

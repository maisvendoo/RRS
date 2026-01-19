#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

class Gizmo;
class IntersectionHandler;
class Outline;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class Group;
class MatrixTransform;
class Viewer;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        const settings_t& settings,
        vsg::ref_ptr<IntersectionHandler> intersection_handler,
        vsg::ref_ptr<vsg::Group> gui_group,
        vsg::observer_ptr<vsg::Viewer> observer_viewer
    );

    void apply(vsg::ButtonPressEvent& buttonPress) override;

private:
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<vsg::Group> gui_group;
    vsg::ref_ptr<vsg::MatrixTransform> object;
    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<Outline> outline;
};

#endif // OBJECT_SELECTOR_H

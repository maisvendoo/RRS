#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include "SelectedObjectsMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

class Gizmo;
class IntersectionHandler;
class KeyboardHandler;
class Route;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MatrixTransform;
class MoveEvent;
class PagedLOD;
class Switch;
class Viewer;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        const settings_t& settings,
        vsg::ref_ptr<KeyboardHandler> keyboard_handler,
        vsg::ref_ptr<IntersectionHandler> intersection_handler,
        vsg::ref_ptr<Route> route,
        vsg::observer_ptr<vsg::Viewer> observer_viewer
    );

    ~ObjectSelector();

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;

    void select_object(
        vsg::ref_ptr<vsg::MatrixTransform> object,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod
    );

    SelectedObjectsIterator deselect_object(
        vsg::ref_ptr<vsg::MatrixTransform> object
    );

private:
    const settings_t& settings;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<Route> route;
    vsg::observer_ptr<vsg::Viewer> observer_viewer;
    SelectedObjectsMap selected_objects;
    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<vsg::Switch> gizmo_switch;
};

#endif // OBJECT_SELECTOR_H

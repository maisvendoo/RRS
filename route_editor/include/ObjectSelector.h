#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Switch.h>

#include <map>

class Gizmo;
class IntersectionHandler;
class KeyboardHandler;
class Route;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MoveEvent;
class PagedLOD;
class Viewer;

}

using MatTrans = vsg::MatrixTransform;
using MatTransPtr = vsg::ref_ptr<MatTrans>;
using GuiSwitch = vsg::Switch;
using GuiSwitchPtr = vsg::ref_ptr<GuiSwitch>;
using SelectedObjectsMap = std::map<MatTransPtr, GuiSwitchPtr>;
using SelectedObjectIterator = SelectedObjectsMap::iterator;

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
        MatTransPtr object,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod
    );

    SelectedObjectIterator deselect_object(MatTransPtr object);

private:
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<Route> route;
    vsg::observer_ptr<vsg::Viewer> observer_viewer;
    SelectedObjectsMap selected_objects;
    vsg::ref_ptr<Gizmo> gizmo;
    GuiSwitchPtr gizmo_switch;
};

#endif // OBJECT_SELECTOR_H

#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Outline.h"
#include "Route.h"

#include <vsg/ui/PointerEvent.h>

#include <cassert>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<Route> route,
    vsg::ref_ptr<vsg::Group> gui_group,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : intersection_handler(intersection_handler)
    , route(route)
    , gui_group(gui_group)
{
    assert(intersection_handler);
    assert(route);
    assert(gui_group);
    assert(observer_viewer);

    gizmo = Gizmo::create(settings);
    outline = Outline::create(observer_viewer);

    gui_group->addChild(gizmo);
    gui_group->addChild(outline);
}

ObjectSelector::~ObjectSelector() = default;

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }


}

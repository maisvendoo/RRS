#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Outline.h"

#include <vsg/ui/PointerEvent.h>

#include <cassert>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<vsg::Group> gui_group,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : intersection_handler(intersection_handler)
    , gui_group(gui_group)
{
    assert(intersection_handler);
    assert(gui_group);
    assert(observer_viewer);

    gizmo = Gizmo::create(settings);
    outline = Outline::create(observer_viewer);

    gui_group->addChild(gizmo);
    gui_group->addChild(outline);
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }


}

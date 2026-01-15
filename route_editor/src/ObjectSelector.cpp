#include "ObjectSelector.h"

#include "Gizmo.h"
#include "Outline.h"
#include "Settings.h"

#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<vsg::Options> options,
    vsg::observer_ptr<vsg::Viewer> observer_viewer,
    vsg::ref_ptr<vsg::Group> gui_group
)
{
    // gizmo = Gizmo::create(settings);

    outline = Outline::create(observer_viewer);

    // gui_group->addChild(gizmo);
    gui_group->addChild(outline);
}

bool ObjectSelector::handle_intersection(const vsg::LineSegmentIntersector::Intersection& intersection)
{
    // active_gizmo_axis = gizmo->handle_intersection(intersection);
    // if (active_gizmo_axis != GizmoAxis::NONE)
    // {
        // return true;
    // }

    return false;
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    // if (active_gizmo_axis == GizmoAxis::NONE)
    // {
        // return;
    // }


}

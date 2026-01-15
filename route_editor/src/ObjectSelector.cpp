#include "ObjectSelector.h"

#include "Gizmo.h"
#include "Outline.h"

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::observer_ptr<vsg::Viewer> observer_viewer,
    vsg::ref_ptr<vsg::Group> gui_group
)
    : gui_group(gui_group)
{
    gizmo = Gizmo::create(settings);
    outline = Outline::create(observer_viewer);

    gui_group->addChild(gizmo);
    gui_group->addChild(outline);
}

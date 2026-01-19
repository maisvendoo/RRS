#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Outline.h"
#include "Route.h"

#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/Switch.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

#include <cassert>

// Если нажимаем на экран, возможны следующие ситуации:
// 1) Ничего не выделено
//    -> Выделения и соответствующего GUI нет
// 2) Ничего не выделено, нажимаем на объект
//    -> Объект нужно выделить и показать GUI
// 3) Ничего не выделено, нажимаем на пустоту
//    -> Ничего не происходит
// 4) Выделен объект, нажимаем на него же
//    -> Выделение снимается, GUI убирается
// 5) Выделен объект, нажимаем на другой объект
//    -> Выделяется другой объект
// 6) Выделен объект, нажимаем на пустоту
//    -> Выделение снимается, GUI убирается
// 7) Выделен объект, нажимаем на Gizmo
//    -> Управляем объектом через Gizmo, пока не отпустим ЛКМ

// TODO: Multiple object selection

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<Route> route,
    vsg::ref_ptr<vsg::Group> gui_group,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : intersection_handler(intersection_handler)
    , route(route)
{
    assert(intersection_handler);
    assert(route);
    assert(gui_group);

    gizmo = Gizmo::create(settings);
    outline = Outline::create(observer_viewer);

    gui_switch = vsg::Switch::create();
    gui_switch->addChild(false, gizmo);
    gui_switch->addChild(false, outline);

    gui_group->addChild(gui_switch);

    scene_switch = vsg::Switch::create();
    scene_switch->addChild(false, gizmo);
    scene_switch->addChild(false, outline);
}

ObjectSelector::~ObjectSelector() = default;

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    const auto lmb_intersector = intersection_handler->get_lmb_intersector();
    if (!lmb_intersector)
    {
        return;
    }

    if (object)
    {
        if (gizmo->handle_intersections(lmb_intersector))
        {
            return;
        }

        route->accept(*lmb_intersector);

        auto& intersections = lmb_intersector->intersections;
        if (intersections.empty())
        {

        }
    }
    else
    {

    }

    route->accept(*lmb_intersector);

    auto& intersections = lmb_intersector->intersections;
    if (intersections.empty())
    {
        return;
    }

    intersection_handler->sort_intersections(lmb_intersector);

    const auto intersection = intersections.front();

    const auto& node_path = intersection->nodePath;
    if (node_path.empty())
    {
        return;
    }

    for (const vsg::Node* const node : node_path)
    {
        const auto matrix_transform = vsg::ref_ptr(
            const_cast<vsg::MatrixTransform*>(
                node->cast<vsg::MatrixTransform>()));

        if (!matrix_transform)
        {
            continue;
        }

        object = matrix_transform;

        break;
    }

    if (!object)
    {
        return;
    }


}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    (void)buttonRelease;
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    (void)moveEvent;
}

void ObjectSelector::select_object(vsg::ref_ptr<vsg::MatrixTransform> object)
{
    this->object = object;
}

void ObjectSelector::deselect_object()
{
}

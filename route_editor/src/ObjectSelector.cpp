#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Outline.h"
#include "Route.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

#include <algorithm>
#include <cassert>

// Если нажимаем на экран, возможны следующие ситуации:
// 1) Ничего не выделено
//    -> Выделения и соответствующего GUI нет
// 2) Ничего не было выделено, нажали на объект
//    -> Объект нужно выделить и показать GUI
// 3) Выделен объект, нажимаем на него же
//    -> Выделение снимается, GUI убирается
// 4) Выделен объект, нажимаем на другой объект
//    -> Выделяется другой объект
// 5) Выделен объект, нажимаем на пустоту
//    -> Выделение снимается, GUI убирается
// 6) Выделен объект, нажимаем на Gizmo
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

    const auto lmb_intersector = intersection_handler->get_lmb_intersector();
    if (!lmb_intersector)
    {
        return;
    }

    route->accept(*lmb_intersector);

    auto& intersections = lmb_intersector->intersections;
    if (intersections.empty())
    {
        return;
    }

    std::sort(intersections.begin(), intersections.end(),
        [](const auto& lhs, const auto& rhs) -> bool
        {
            return (lhs->ratio) < (rhs->ratio);
        }
    );

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

#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "Outline.h"
#include "Route.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
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
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<Route> route,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : keyboard_handler(keyboard_handler)
    , intersection_handler(intersection_handler)
    , route(route)
    , observer_viewer(observer_viewer)
{
    assert(intersection_handler);
    assert(route);

    gizmo = Gizmo::create(settings, selected_objects);

    gizmo_switch = GuiSwitch::create();
    gizmo_switch->addChild(vsg::MASK_OFF, gizmo);

    route->addChild(gizmo_switch);
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

    const bool are_selected_objects_empty = selected_objects.empty();

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!are_selected_objects_empty &&
        gizmo->handle_intersections(lmb_intersector))
    {
        return;
    }

    route->accept(*lmb_intersector);

    auto& intersections = lmb_intersector->intersections;
    if (intersections.empty())
    {
        // If we clicked on empty space without shift
        // while there were selected objects,
        // deselect them all
        if (!are_selected_objects_empty && !keyboard_handler->get_shift_state())
        {
            for (auto it = selected_objects.begin();
                it != selected_objects.end();
                it = deselect_object(it->first));
        }

        return;
    }

    intersection_handler->sort_intersections(lmb_intersector);

    const auto& node_path = intersections.front()->nodePath;
    assert(!node_path.empty());

    for (const vsg::Node* const node : node_path)
    {
        const auto matrix_transform = vsg::ref_ptr(const_cast<MatTrans*>(
            node->cast<MatTrans>()));

        if (!matrix_transform)
        {
            continue;
        }

        const auto& mt_children = matrix_transform->children;
        if (mt_children.empty())
        {
            continue;
        }

        const auto paged_lod_switch = mt_children[0].cast<vsg::Switch>();
        if (!paged_lod_switch)
        {
            continue;
        }

        const auto& pl_switch_children = paged_lod_switch->children;
        if (pl_switch_children.empty())
        {
            continue;
        }

        const auto paged_lod = pl_switch_children[0].node.cast<vsg::PagedLOD>();
        if (paged_lod)
        {
            select_object(matrix_transform, paged_lod);
            break;
        }
    }

    intersections.clear();
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    (void)buttonRelease;
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    (void)moveEvent;
}

void ObjectSelector::select_object(
    MatTransPtr object,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod
)
{
    assert(object);
    assert(paged_lod);

    const vsg::ref_ptr<vsg::Viewer> viewer = observer_viewer;
    const bool is_shift = keyboard_handler->get_shift_state();
    const bool selected_objects_were_empty = selected_objects.empty();

    const auto found_it = selected_objects.find(object);
    const bool clicked_on_selected_object = (found_it != selected_objects.end());

    if (is_shift)
    {
        if (clicked_on_selected_object)
        {
            deselect_object(object);
            return;
        }
        else
        {
            const auto outline = Outline::create(paged_lod, observer_viewer);

            const auto outline_switch = vsg::Switch::create();
            outline_switch->addChild(vsg::Mask{MASK_GUI}, outline);

            object->addChild(outline_switch);

            selected_objects[object] = outline_switch;

            return;
        }
    }

    // if (selected_object)
    // {
    //     deselect_object(selected_object);
    // }

    // if (selected_object == object)
    // {
    //     selected_object = nullptr;
    //     return;
    // }

    // selected_object = object;

    // const auto compile_result = viewer->compileManager->compile(switch_group);
    // selected_object->addChild(switch_group);

    // gizmo->set_outer_matrix(&selected_object->matrix);

    // const auto pl_switch = selected_object->children[0].cast<vsg::Switch>();
    // const auto paged_lod = pl_switch->children[0].node.cast<vsg::PagedLOD>();
    // outline->update(paged_lod);

    // for (auto& child : switch_group->children)
    // {
    //     child.mask = MASK_GUI;
    // }

    // vsg::updateViewer(*viewer, compile_result);
}

SelectedObjectIterator ObjectSelector::deselect_object(MatTransPtr object)
{
    assert(object);

    const bool is_shift = keyboard_handler->get_shift_state();

    // auto& children = object->children;
    // assert(!children.empty());

    // for (auto it = children.begin(); it != children.end(); ++it)
    // {
    //     if (*it == switch_group)
    //     {
    //         children.erase(it);
    //         break;
    //     }
    // }

    // gizmo->set_outer_matrix(nullptr);

    // for (auto& child : switch_group->children)
    // {
    //     child.mask = vsg::MASK_OFF;
    // }
}

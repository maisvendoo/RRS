#include "ObjectSelector.h"

#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "Outline.h"
#include "Route.h"
#include "SelectedObjectsMap.h"

#include <vsg/app/CompileManager.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Switch.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

#include <cassert>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    vsg::ref_ptr<vsg::LookAt> look_at,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<Route> route,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : settings(settings)
    , keyboard_handler(keyboard_handler)
    , intersection_handler(intersection_handler)
    , route(route)
    , observer_viewer(observer_viewer)
{
    assert(intersection_handler);
    assert(route);

    gizmo = Gizmo::create(settings, look_at, selected_objects);

    const auto gizmo_switch = vsg::Switch::create();
    gizmo_switch->addChild(vsg::MASK_OFF, gizmo);

    gizmo_mask = &gizmo_switch->children[0].mask;
    assert(gizmo_mask);

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

    const bool selected_objects_are_empty = selected_objects.empty();

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects_are_empty &&
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
        if (!selected_objects_are_empty && !keyboard_handler->get_shift_state())
        {
            for (auto it = selected_objects.begin();
                it != selected_objects.end();
                it = deselect_object(it->first));
        }

        *gizmo_mask = selected_objects.empty()
            ? vsg::MASK_OFF
            : MASK_GUI | MASK_CLICKABLE;

        return;
    }

    intersection_handler->sort_intersections(lmb_intersector);

    const auto& node_path = intersections.front()->nodePath;
    assert(!node_path.empty());

    for (const vsg::Node* const node : node_path)
    {
        const auto matrix_transform = vsg::ref_ptr(
            const_cast<vsg::MatrixTransform*>(
                node->cast<vsg::MatrixTransform>()));

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

    if (selected_objects.empty())
    {
        *gizmo_mask = vsg::MASK_OFF;
    }
    else
    {
        gizmo->update_position();
        *gizmo_mask = MASK_GUI | MASK_CLICKABLE;
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

void ObjectSelector::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    gizmo->update_scale();
}

void ObjectSelector::select_object(
    vsg::ref_ptr<vsg::MatrixTransform> object,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod
)
{
    assert(object);
    assert(paged_lod);

    const bool clicked_on_selected_object = (
        selected_objects.find(object) != selected_objects.end());

    if (keyboard_handler->get_shift_state())
    {
        if (clicked_on_selected_object)
        {
            deselect_object(object);
        }
        else
        {
            select_object_inner(object, paged_lod);
        }
    }
    else
    {
        if (selected_objects.empty())
        {
            select_object_inner(object, paged_lod);
        }
        else if (clicked_on_selected_object)
        {
            if (selected_objects.size() == 1)
            {
                deselect_object(object);
            }
            else
            {
                for (auto it = selected_objects.begin();
                    it != selected_objects.end();)
                {
                    if (it->first == object)
                    {
                        ++it;
                    }
                    else
                    {
                        it = deselect_object(it->first);
                    }
                }
            }
        }
        else
        {
            for (auto it = selected_objects.begin();
                it != selected_objects.end();
                it = deselect_object(it->first));

            select_object_inner(object, paged_lod);
        }
    }
}

void ObjectSelector::select_object_inner(
    vsg::ref_ptr<vsg::MatrixTransform> object,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod
)
{
    const auto outline = Outline::create(settings, paged_lod, observer_viewer);

    const auto outline_switch = vsg::Switch::create();
    outline_switch->addChild(vsg::Mask{MASK_GUI}, outline);

    const vsg::ref_ptr<vsg::Viewer> viewer = observer_viewer;

    const vsg::CompileResult compile_result =
            viewer->compileManager->compile(outline_switch);

    object->addChild(outline_switch);

    vsg::updateViewer(*viewer, compile_result);

    selected_objects[object] = outline_switch;
}

SelectedObjectsIterator ObjectSelector::deselect_object(
    vsg::ref_ptr<vsg::MatrixTransform> object
)
{
    assert(object);
    assert(selected_objects.count(object));

    const auto outline_switch = selected_objects[object];

    for (auto it = object->children.begin(); it != object->children.end(); ++it)
    {
        if (*it == outline_switch)
        {
            object->children.erase(it);

            break;
        }
    }

    return selected_objects.erase(selected_objects.find(object));
}

#include "ObjectSelector.h"

#include "CameraHandler.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "Outline.h"
#include "SceneGraph.h"
#include "SelectedObjectsMap.h"
#include "SingleSwitch.h"
#include "SwitchGroup.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

#include <cassert>

ObjectSelector::ObjectSelector(
    const settings_t& settings,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    vsg::ref_ptr<CameraHandler> camera_handler,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    vsg::ref_ptr<SceneGraph> scene_graph,
    vsg::observer_ptr<vsg::Viewer> observer_viewer
)
    : settings(settings)
    , keyboard_handler(keyboard_handler)
    , intersection_handler(intersection_handler)
    , scene_graph(scene_graph)
    , observer_viewer(observer_viewer)
{
    assert(keyboard_handler);
    assert(camera_handler);
    assert(intersection_handler);
    assert(scene_graph);
    assert(observer_viewer);

    gizmo = Gizmo::create(settings, camera_handler,
        intersection_handler, selected_objects);

    gizmo_switch = SingleSwitch::create(vsg::MASK_OFF, gizmo);

    scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE}, gizmo_switch);
}

void ObjectSelector::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    const bool selected_objects_are_empty = selected_objects.empty();

    // If we have selected objects and clicked on Gizmo,
    // handle Gizmo intersection (start moving objects with Gizmo)
    if (!selected_objects_are_empty && gizmo->handle_intersections())
    {
        return;
    }

    const auto intersector = intersection_handler->get_lmb_intersector();
    if (!intersector)
    {
        return;
    }

    scene_graph->accept(*intersector);

    auto& intersections = intersector->intersections;
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

        gizmo_switch->mask = selected_objects.empty()
            ? vsg::MASK_OFF
            : MASK_GUI1 | MASK_CLICKABLE;

        return;
    }

    intersection_handler->sort_intersections(intersections);

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

        const auto switch_group = mt_children.front().cast<SwitchGroup>();
        if (!switch_group)
        {
            continue;
        }

        const auto& sg_children = switch_group->children;
        if (sg_children.empty())
        {
            continue;
        }

        const auto paged_lod = sg_children.front().node.cast<vsg::PagedLOD>();
        if (paged_lod)
        {
            select_object(matrix_transform, switch_group, paged_lod);

            break;
        }
    }

    intersections.clear();

    if (selected_objects.empty())
    {
        gizmo_switch->mask = vsg::MASK_OFF;
    }
    else
    {
        gizmo->update_position();
        gizmo_switch->mask = MASK_GUI1 | MASK_CLICKABLE;
    }
}

void ObjectSelector::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    gizmo->apply(buttonRelease);
}

void ObjectSelector::apply(vsg::MoveEvent& moveEvent)
{
    gizmo->apply(moveEvent);
}

void ObjectSelector::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    gizmo->update_scale();
}

void ObjectSelector::select_object(
    vsg::ref_ptr<vsg::MatrixTransform> object,
    vsg::ref_ptr<SwitchGroup> switch_group,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod
)
{
    assert(object);
    assert(switch_group);
    assert(paged_lod);

    const bool clicked_on_selected_object = (
        selected_objects.find(object) != selected_objects.end());

    const auto select_object_inner = [&]() -> void
    {
        const auto viewer = observer_viewer.ref_ptr();
        const auto compile_manager = viewer->compileManager;

        const auto outline = Outline::create(settings, paged_lod);

        const auto compile_result = compile_manager->compile(outline);

        switch_group->addChild(vsg::Mask{MASK_GUI2}, outline);

        vsg::updateViewer(*viewer, compile_result);

        selected_objects[object] = {switch_group, outline};
    };

    if (keyboard_handler->get_shift_state())
    {
        if (clicked_on_selected_object)
        {
            deselect_object(object);
        }
        else
        {
            select_object_inner();
        }
    }
    else
    {
        if (selected_objects.empty())
        {
            select_object_inner();
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

            select_object_inner();
        }
    }
}

SelectedObjectsIterator ObjectSelector::deselect_object(
    vsg::ref_ptr<vsg::MatrixTransform> object
)
{
    assert(object);
    assert(selected_objects.count(object));

    const auto switch_group = selected_objects[object].switch_group;
    auto& sg_children = switch_group->children;

    const auto outline = selected_objects[object].outline;

    for (auto it = sg_children.begin(); it != sg_children.end(); ++it)
    {
        if (it->node == outline)
        {
            sg_children.erase(it);

            break;
        }
    }

    return selected_objects.erase(selected_objects.find(object));
}

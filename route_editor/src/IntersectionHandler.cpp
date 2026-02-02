#include "IntersectionHandler.h"

#include "LSIntersector.h"
#include "Mask.h"
#include "MouseButton.h"

#include <vsg/app/Camera.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>
#include <vsg/ui/PointerEvent.h>

#include <algorithm>
#include <cassert>

IntersectionHandler::IntersectionHandler(vsg::ref_ptr<vsg::Camera> camera)
    : camera(camera)
{
    assert(camera);
}

void IntersectionHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    switch (buttonPress.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            lmb_intersector = LSIntersector::create(*camera,
                buttonPress.x, buttonPress.y);

            lmb_intersector->traversalMask = MASK_CLICKABLE;

            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector = LSIntersector::create(*camera,
                buttonPress.x, buttonPress.y);

            mmb_intersector->traversalMask = MASK_CLICKABLE;

            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector = LSIntersector::create(*camera,
                buttonPress.x, buttonPress.y);

            rmb_intersector->traversalMask = MASK_CLICKABLE;

            return;
        }
        default:
        {
            return;
        }
    }
}

void IntersectionHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    switch (buttonRelease.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            lmb_intersector = nullptr;

            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector = nullptr;

            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector = nullptr;

            return;
        }
        default:
        {
            return;
        }
    }
}

LSIntersectorRefPtr IntersectionHandler::apply_(
    const vsg::MoveEvent& moveEvent
) const
{
    return LSIntersector::create(*camera, moveEvent.x, moveEvent.y);
}

LSIntersectorRefPtr IntersectionHandler::apply_(vsg::ivec2 mouse_pos) const
{
    return LSIntersector::create(*camera, mouse_pos.x, mouse_pos.y);
}

LSIntersectorRefPtr IntersectionHandler::get_lmb_intersector() const
{
    return lmb_intersector;
}

LSIntersectorRefPtr IntersectionHandler::get_mmb_intersector() const
{
    return mmb_intersector;
}

LSIntersectorRefPtr IntersectionHandler::get_rmb_intersector() const
{
    return rmb_intersector;
}

void IntersectionHandler::sort_intersections(LSIntersectorRefPtr intersector)
{
    sort_intersections(intersector->intersections);
}

void IntersectionHandler::sort_intersections(LSIntersections& intersections)
{
    if (intersections.empty())
    {
        return;
    }

    std::sort(intersections.begin(), intersections.end(),
        [](const auto& lhs, const auto& rhs) -> bool {
            return (lhs->ratio) < (rhs->ratio);
        }
    );
}

LSIntersectionRefPtr IntersectionHandler::get_closest_intersection(
    LSIntersectorRefPtr intersector)
{
    if (!intersector)
    {
        return nullptr;
    }

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        return nullptr;
    }

    sort_intersections(intersections);

    return intersections.front();
}

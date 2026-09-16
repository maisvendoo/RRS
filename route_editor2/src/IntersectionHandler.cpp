#include "editor/IntersectionHandler.h"

#include "editor/Mask.h"
#include "editor/MouseButton.h"

#include <vsg/app/Camera.h>
#include <vsg/maths/vec2.h>
#include <vsg/ui/PointerEvent.h>

#include <algorithm>

IntersectionHandler::IntersectionHandler(vsg::ref_ptr<vsg::Camera> camera_)
    : camera(camera_)
{
}

void IntersectionHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    const auto intersector = LSIntersector::create(*camera,
        buttonPress.x, buttonPress.y);

    intersector->traversalMask = editor2::MASK_CLICKABLE;

    switch (buttonPress.button)
    {
        case editor2::MOUSE_BUTTON_LEFT:
        {
            lmb_intersector_ = intersector;
            return;
        }
        case editor2::MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector_ = intersector;
            return;
        }
        case editor2::MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector_ = intersector;
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
        case editor2::MOUSE_BUTTON_LEFT:
        {
            lmb_intersector_ = nullptr;
            return;
        }
        case editor2::MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector_ = nullptr;
            return;
        }
        case editor2::MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector_ = nullptr;
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
    return lmb_intersector_;
}

LSIntersectorRefPtr IntersectionHandler::get_mmb_intersector() const
{
    return mmb_intersector_;
}

LSIntersectorRefPtr IntersectionHandler::get_rmb_intersector() const
{
    return rmb_intersector_;
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
    LSIntersectorRefPtr intersector
)
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

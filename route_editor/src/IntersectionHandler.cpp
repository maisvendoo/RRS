#include "IntersectionHandler.h"

#include "EditorContext.h"
#include "LSIntersector.h"
#include "Mask.h"

#include <vsg/app/Camera.h>
#include <vsg/maths/vec2.h>
#include <vsg/ui/PointerEvent.h>

#include <algorithm>

IntersectionHandler::IntersectionHandler(const vsg::ref_ptr<vsg::Camera>& camera)
    : camera(camera)
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

    intersector->traversalMask = MASK_CLICKABLE;

    if (buttonPress.mask & vsg::BUTTON_MASK_1)
    {
        lmb_intersector_ = intersector;
    }

    if (buttonPress.mask & vsg::BUTTON_MASK_2)
    {
        mmb_intersector_ = intersector;
    }

    if (buttonPress.mask & vsg::BUTTON_MASK_3)
    {
        rmb_intersector_ = intersector;
    }
}

void IntersectionHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    if (!(buttonRelease.mask & vsg::BUTTON_MASK_1))
    {
        lmb_intersector_ = nullptr;
    }

    if (!(buttonRelease.mask & vsg::BUTTON_MASK_2))
    {
        mmb_intersector_ = nullptr;
    }

    if (!(buttonRelease.mask & vsg::BUTTON_MASK_3))
    {
        rmb_intersector_ = nullptr;
    }
}

LSIntersectorRefPtr IntersectionHandler::apply_(
    const vsg::MoveEvent& moveEvent
) const
{

    return LSIntersector::create(*camera, moveEvent.x, moveEvent.y);
}

LSIntersectorRefPtr IntersectionHandler::apply_(int mouse_x, int mouse_y) const
{
    return LSIntersector::create(*camera, mouse_x, mouse_y);
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

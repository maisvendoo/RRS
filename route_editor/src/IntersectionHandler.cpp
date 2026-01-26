#include "IntersectionHandler.h"

#include "Mask.h"
#include "MouseButton.h"

#include <vsg/app/Camera.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

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
            lmb_intersector = vsg::LineSegmentIntersector::create(
                *camera, buttonPress.x, buttonPress.y);

            lmb_intersector->traversalMask = MASK_CLICKABLE;

            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector = vsg::LineSegmentIntersector::create(
                *camera, buttonPress.x, buttonPress.y);

            mmb_intersector->traversalMask = MASK_CLICKABLE;

            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector = vsg::LineSegmentIntersector::create(
                *camera, buttonPress.x, buttonPress.y);

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

vsg::ref_ptr<vsg::LineSegmentIntersector>
IntersectionHandler::get_lmb_intersector() const
{
    return lmb_intersector;
}

vsg::ref_ptr<vsg::LineSegmentIntersector>
IntersectionHandler::get_mmb_intersector() const
{
    return mmb_intersector;
}

vsg::ref_ptr<vsg::LineSegmentIntersector>
IntersectionHandler::get_rmb_intersector() const
{
    return rmb_intersector;
}

void IntersectionHandler::sort_intersections(
    vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
)
{
    sort_intersections(intersector->intersections);
}

void IntersectionHandler::sort_intersections(
    vsg::LineSegmentIntersector::Intersections& intersections
)
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

#include "IntersectionHandler.h"

#include "MouseButton.h"

#include <vsg/app/Camera.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

IntersectionHandler::IntersectionHandler(vsg::ref_ptr<vsg::Camera> camera)
    : camera(camera)
{
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

            break;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector = vsg::LineSegmentIntersector::create(
                *camera, buttonPress.x, buttonPress.y);

            break;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector = vsg::LineSegmentIntersector::create(
                *camera, buttonPress.x, buttonPress.y);

            break;
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
            break;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_intersector = nullptr;
            break;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_intersector = nullptr;
            break;
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

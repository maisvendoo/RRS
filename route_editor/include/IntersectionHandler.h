#ifndef INTERSECTION_HANDLER_H
#define INTERSECTION_HANDLER_H

#include "LSIntersector.h"

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>

struct EditorContext;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MoveEvent;

}

class IntersectionHandler : public vsg::Inherit<
    vsg::Visitor, IntersectionHandler>
{
public:
    explicit IntersectionHandler(vsg::ref_ptr<vsg::Camera>& camera);

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    LSIntersectorRefPtr apply_(const vsg::MoveEvent& moveEvent) const;
    LSIntersectorRefPtr apply_(vsg::ivec2 mouse_pos) const;

    LSIntersectorRefPtr get_lmb_intersector() const;
    LSIntersectorRefPtr get_mmb_intersector() const;
    LSIntersectorRefPtr get_rmb_intersector() const;

    static void sort_intersections(LSIntersectorRefPtr intersector);
    static void sort_intersections(LSIntersections& intersections);

    static LSIntersectionRefPtr get_closest_intersection(
        LSIntersectorRefPtr intersector);

private:
    vsg::ref_ptr<vsg::Camera>& camera;

    LSIntersectorRefPtr lmb_intersector_;
    LSIntersectorRefPtr mmb_intersector_;
    LSIntersectorRefPtr rmb_intersector_;
};

#endif // INTERSECTION_HANDLER_H

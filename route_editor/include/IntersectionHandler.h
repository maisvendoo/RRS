#ifndef INTERSECTION_HANDLER_H
#define INTERSECTION_HANDLER_H

#include "LSIntersector.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
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
    explicit IntersectionHandler(const EditorContext& context);

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
    const EditorContext& context;

    LSIntersectorRefPtr lmb_intersector;
    LSIntersectorRefPtr mmb_intersector;
    LSIntersectorRefPtr rmb_intersector;
};

#endif // INTERSECTION_HANDLER_H

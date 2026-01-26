#ifndef INTERSECTION_HANDLER_H
#define INTERSECTION_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/utils/LineSegmentIntersector.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class Camera;

}

class IntersectionHandler : public vsg::Inherit<
    vsg::Visitor, IntersectionHandler>
{
public:
    explicit IntersectionHandler(vsg::ref_ptr<vsg::Camera> camera);

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    vsg::ref_ptr<vsg::LineSegmentIntersector> get_lmb_intersector() const;
    vsg::ref_ptr<vsg::LineSegmentIntersector> get_mmb_intersector() const;
    vsg::ref_ptr<vsg::LineSegmentIntersector> get_rmb_intersector() const;

    static void sort_intersections(
        vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
    );

    static void sort_intersections(
        vsg::LineSegmentIntersector::Intersections& intersections
    );

private:
    vsg::ref_ptr<vsg::Camera> camera;

    vsg::ref_ptr<vsg::LineSegmentIntersector> lmb_intersector;
    vsg::ref_ptr<vsg::LineSegmentIntersector> mmb_intersector;
    vsg::ref_ptr<vsg::LineSegmentIntersector> rmb_intersector;
};

#endif // INTERSECTION_HANDLER_H

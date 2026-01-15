#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include "Gizmo.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/utils/LineSegmentIntersector.h>

class Outline;
struct settings_t;

namespace vsg
{

class Group;
class MatrixTransform;
class MoveEvent;
class Options;
class Viewer;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options,
        vsg::observer_ptr<vsg::Viewer> observer_viewer,
        vsg::ref_ptr<vsg::Group> gui_group
    );

    bool handle_intersection(const vsg::LineSegmentIntersector::Intersection& intersection);

    void apply(vsg::MoveEvent& moveEvent) override;

public:
    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<Outline> outline;

private:
    vsg::ref_ptr<vsg::MatrixTransform> object;
    // GizmoAxis active_gizmo_axis = GizmoAxis::NONE;

    vsg::dvec3 translation;
    vsg::dvec3 rotation;
    vsg::dvec3 scale;
};

#endif // OBJECT_SELECTOR_H

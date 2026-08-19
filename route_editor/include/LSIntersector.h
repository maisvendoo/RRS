#ifndef LS_INTERSECTOR_H
#define LS_INTERSECTOR_H

#include <vsg/core/ref_ptr.h>
#include <vsg/utils/LineSegmentIntersector.h>

using LSIntersector = vsg::LineSegmentIntersector;
using LSIntersectorRefPtr = vsg::ref_ptr<LSIntersector>;

using LSIntersection = LSIntersector::Intersection;
using LSIntersectionRefPtr = vsg::ref_ptr<LSIntersection>;

using LSIntersections = LSIntersector::Intersections;

#endif // LS_INTERSECTOR_H

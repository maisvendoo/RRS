#ifndef ROUTE_OBJECTS_H
#define ROUTE_OBJECTS_H

#include <vsg/core/ref_ptr.h>

#include <list>

class RouteObject;

using RouteObjects = std::list<vsg::ref_ptr<RouteObject>>;
using RouteObjectsIterator = RouteObjects::iterator;

#endif // ROUTE_OBJECTS_H

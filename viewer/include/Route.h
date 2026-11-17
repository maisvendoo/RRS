#ifndef VIEWER_ROUTE_H
#define VIEWER_ROUTE_H

#include <RouteReader/ObjectsRef.h>
#include <RouteReader/RouteMap.h>

struct Route
{
    ObjectsRef objects_ref;
    RouteMap route_map;
};

#endif // VIEWER_ROUTE_H

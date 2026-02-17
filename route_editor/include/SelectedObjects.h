#ifndef SELECTED_OBJECTS_H
#define SELECTED_OBJECTS_H

#include <vsg/core/ref_ptr.h>

#include <list>

class RouteObject;

using SelectedObjects = std::list<vsg::ref_ptr<RouteObject>>;
using SelectedObjectsIterator = SelectedObjects::iterator;

#endif // SELECTED_OBJECTS_H

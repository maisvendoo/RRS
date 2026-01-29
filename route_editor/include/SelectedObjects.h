#ifndef SELECTED_OBJECTS_H
#define SELECTED_OBJECTS_H

#include <vsg/core/ref_ptr.h>

#include <vector>

class RouteObject;

using SelectedObjects = std::vector<vsg::ref_ptr<RouteObject>>;
using SelectedObjectsIterator = SelectedObjects::iterator;

#endif // SELECTED_OBJECTS_H

#ifndef SELECTED_OBJECTS_MAP_H
#define SELECTED_OBJECTS_MAP_H

#include <vsg/core/ref_ptr.h>

#include <map>

namespace vsg
{

class MatrixTransform;
class Switch;

}

using SelectedObjectsMap = std::map<vsg::ref_ptr<vsg::MatrixTransform>,
    vsg::ref_ptr<vsg::Switch>>;

using SelectedObjectsIterator = SelectedObjectsMap::iterator;

#endif // SELECTED_OBJECTS_MAP_H

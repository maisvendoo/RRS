#ifndef SELECTED_OBJECTS_MAP_H
#define SELECTED_OBJECTS_MAP_H

#include <vsg/core/ref_ptr.h>

#include <map>

class Outline;

namespace vsg
{

class MatrixTransform;
class Switch;

}

struct SelectedObjectData
{
    vsg::ref_ptr<vsg::Switch> switch_group;
    vsg::ref_ptr<Outline> outline;
};

using SelectedObjectsMap = std::map<vsg::ref_ptr<vsg::MatrixTransform>,
    SelectedObjectData>;

using SelectedObjectsIterator = SelectedObjectsMap::iterator;

#endif // SELECTED_OBJECTS_MAP_H

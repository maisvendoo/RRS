#ifndef EDITOR_OBJECT_MANAGER_H
#define EDITOR_OBJECT_MANAGER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>

#include <cstddef>
#include <string>
#include <vector>

namespace vsg
{

class MatrixTransform;

}

class ObjectManager
{
public:
    std::vector<std::string> labels;
    std::vector<std::string> relative_paths;
    std::vector<vsg::ref_ptr<vsg::MatrixTransform>> matrix_transforms;
    std::vector<vsg::dmat4> initial_matrixes;
    std::vector<bool> is_selected;

public:
    ObjectManager(std::size_t max_object_count);
};

#endif // EDITOR_OBJECT_MANAGER_H

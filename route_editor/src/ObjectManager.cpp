#include "ObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>

#include <cstddef>

ObjectManager::ObjectManager(std::size_t max_object_count)
{
    labels.reserve(max_object_count);
    relative_paths.reserve(max_object_count);
    matrix_transforms.reserve(max_object_count);
    initial_matrixes.reserve(max_object_count);
    is_selected.reserve(max_object_count);
}

#include "ObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <cstddef>

ObjectManager::ObjectManager(std::size_t max_object_count)
{
    paged_lods.reserve(max_object_count);
    matrix_transforms.reserve(max_object_count);
}

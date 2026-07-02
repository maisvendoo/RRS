#include "editor/ObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <cstddef>

ObjectManager::ObjectManager(std::size_t max_object_count)
    : max_object_count(max_object_count)
{
    paged_lods.reserve(max_object_count);
    transforms.reserve(max_object_count);
}

ObjectManager::~ObjectManager() = default;

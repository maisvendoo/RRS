#include "editor/ObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

ObjectManager::ObjectManager(int max_object_count)
    : max_object_count(max_object_count)
{
    paged_lods.reserve(max_object_count);
    transforms.reserve(max_object_count);
}

ObjectManager::~ObjectManager() = default;

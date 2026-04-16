#include "ObjectManager.h"

#include <cstddef>

ObjectManager::ObjectManager(
    EditorContext& context,
    std::size_t max_object_count
)
    : context_(context)
{
    transforms_.reserve(max_object_count);
    labels_.reserve(max_object_count);
    translations_.reserve(max_object_count);
    rotation_degs_.reserve(max_object_count);
    scales_.reserve(max_object_count);
    initial_matrices_.reserve(max_object_count);
    bounds_.reserve(max_object_count);
    is_selected_.reserve(max_object_count);
    is_hidden_.reserve(max_object_count);
    paged_lod_switches_.reserve(max_object_count);
    paged_lods_.reserve(max_object_count);
    outline_switches_.reserve(max_object_count);
}

ObjectManager::~ObjectManager()
{

}

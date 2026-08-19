#include "commands/TransformObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <cstddef>

TransformObjects::TransformObjects(
    EditorContext& context,
    const RouteObjects& objects
)
    : Command(context)
    , objects_(objects)
{
    initial_matrices_.reserve(objects.size());
    for (const auto& object : objects)
    {
        initial_matrices_.emplace_back(object->get_initial_matrix());
    }
}

void TransformObjects::undo()
{
    std::size_t index = 0;
    for (const auto& object : objects_)
    {
        object->set_matrix(initial_matrices_[index]);
        ++index;
    }
}

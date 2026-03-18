#include "TransformObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <cstddef>

TransformObjectsCommand::TransformObjectsCommand(EditorContext& context)
    : Command(context)
    , objects(context.selected_objects)
{
    initial_matrices.reserve(objects.size());
    for (RouteObject* const object : objects)
    {
        initial_matrices.emplace_back(object->get_initial_matrix());
    }
}

void TransformObjectsCommand::undo() const
{
    std::size_t index = 0;
    for (RouteObject* const object : objects)
    {
        object->matrix = initial_matrices[index];
        object->update_bounds();
        ++index;
    }
}

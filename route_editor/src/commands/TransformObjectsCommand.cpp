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
    for (const auto& object : objects)
    {
        initial_matrices.emplace_back(object->get_initial_matrix());
    }
}

void TransformObjectsCommand::undo()
{
    std::size_t index = 0;
    for (const auto& object : objects)
    {
        object->set_matrix(initial_matrices[index]);
        ++index;
    }
}

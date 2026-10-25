#include "editor/commands/TransformObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <cstddef>

TransformObjectsCommand::TransformObjectsCommand(
    EditorContext& context,
    const RouteObjects& objects
)
    : Command(context)
    , objects(objects)
{
    initial_matrices.reserve(objects.size());

    for (const auto& object : objects)
        initial_matrices.emplace_back(object->get_initial_matrix());
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

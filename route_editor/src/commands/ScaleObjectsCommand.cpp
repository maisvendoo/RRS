#include "ScaleObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstddef>
#include <cstdio>

ScaleObjectsCommand::ScaleObjectsCommand(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 scale)
    : Command(context)
    , objects(context.selected_objects)
    , pivot(pivot)
    , scale(scale)
{
    initial_matrices.reserve(objects.size());
    for (RouteObject* const object : objects)
    {
        initial_matrices.emplace_back(object->matrix);
    }

    update_description();
}

void ScaleObjectsCommand::execute() const
{
    for (RouteObject* const object : objects)
    {
        object->scale_relative_to_pivot(pivot, scale, object->matrix);
    }
}

void ScaleObjectsCommand::undo() const
{
    std::size_t index = 0;
    for (RouteObject* const object : objects)
    {
        object->matrix = initial_matrices[index];
        object->update_bounds();
        ++index;
    }
}

void ScaleObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Scale objects: pivot = { %10.3f, %10.3f, %10.3f }\n"
        "               scale = { %10.3f, %10.3f, %10.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z);
}

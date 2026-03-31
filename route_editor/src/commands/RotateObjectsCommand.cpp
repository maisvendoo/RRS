#include "RotateObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

RotateObjectsCommand::RotateObjectsCommand(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 axis, float radians)
    : TransformObjectsCommand(context)
    , pivot(pivot)
    , axis(axis)
    , radians(radians)
{
    update_description();
}

void RotateObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->rotate_around_pivot(pivot, axis, radians, object->matrix);
    }
}

void RotateObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Rotate objects: pivot = { %.3f, %.3f, %.3f }\n"
        "                 axis = { %.3f, %.3f, %.3f }\n"
        "              radians = %.3f",
        pivot.x, pivot.y, pivot.z, axis.x, axis.y, axis.z, radians
    );
}

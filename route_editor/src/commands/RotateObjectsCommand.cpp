#include "editor/commands/RotateObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"
#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

RotateObjectsCommand::RotateObjectsCommand(
    EditorContext& context,
    const RouteObjects& objects,
    const vsg::dvec3& pivot,
    const vsg::dvec3& axis,
    double radians
)
    : TransformObjectsCommand(context, objects)
    , pivot(pivot)
    , axis(axis)
    , radians(radians)
{
    update_description();
}

void RotateObjectsCommand::execute()
{
    for (const auto& object : objects)
        object->rotate_around_pivot(pivot, axis, radians, object->matrix);
}

void RotateObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Rotate objects: pivot = { %.3f, %.3f, %.3f }\n"
        "                 axis = { %.3f, %.3f, %.3f }\n"
        "              radians = %.3f",
        pivot.x, pivot.y, pivot.z, axis.x, axis.y, axis.z, radians);
}

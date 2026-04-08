#include "commands/RotateObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

RotateObjects::RotateObjects(EditorContext& context,
    const vsg::dvec3& pivot, const vsg::dvec3& axis, double radians)
    : TransformObjects(context)
    , pivot(pivot)
    , axis(axis)
    , radians(radians)
{
    update_description();
}

void RotateObjects::execute()
{
    for (const auto& object : objects)
    {
        object->rotate_around_pivot(pivot, axis, radians, object->matrix);
    }
}

void RotateObjects::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Rotate objects: pivot = { %.3f, %.3f, %.3f }\n"
        "                 axis = { %.3f, %.3f, %.3f }\n"
        "              radians = %.3f",
        pivot.x, pivot.y, pivot.z, axis.x, axis.y, axis.z, radians
    );
}

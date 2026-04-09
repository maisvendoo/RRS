#include "commands/RotateObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

RotateObjects::RotateObjects(EditorContext& context,
    const vsg::dvec3& pivot, const vsg::dvec3& axis, double radians)
    : TransformObjects(context, context.selected_objects)
    , pivot_(pivot)
    , axis_(axis)
    , radians_(radians)
{
    update_description();
}

void RotateObjects::execute()
{
    for (const auto& object : objects_)
    {
        object->rotate_around_pivot(pivot_, axis_, radians_, object->matrix);
    }
}

void RotateObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Rotate objects: pivot = { %.3f, %.3f, %.3f }\n"
        "                 axis = { %.3f, %.3f, %.3f }\n"
        "              radians = %.3f",
        pivot_.x, pivot_.y, pivot_.z, axis_.x, axis_.y, axis_.z, radians_
    );
}

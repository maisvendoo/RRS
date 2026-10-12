#include "editor/commands/ScaleObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"
#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

ScaleObjectsCommand::ScaleObjectsCommand(EditorContext& context, const RouteObjects& objects,
    const vsg::dvec3& pivot, const vsg::dvec3& scale)
    : TransformObjectsCommand(context, objects)
    , pivot(pivot)
    , scale(scale)
{
    update_description();
}

void ScaleObjectsCommand::execute()
{
    for (const auto& object : objects)
        object->scale_relative_to_pivot(pivot, scale, object->matrix);
}

void ScaleObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Scale objects: pivot = { %.3f, %.3f, %.3f }\n"
        "               scale = { %.3f, %.3f, %.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z);
}

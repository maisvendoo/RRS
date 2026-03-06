#include "ScaleObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>
#include <string>

ScaleObjectsCommand::ScaleObjectsCommand(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 scale)
    : Command(context)
    , objects(context.selected_objects)
    , pivot(pivot)
    , scale(scale)
{
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
    for (RouteObject* const object : objects)
    {
        // TODO: Wrong calculation. Fix later
        // (if we scale by zero, we can not undo this)
        object->scale_relative_to_pivot(pivot,
            vsg::vec3(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z),
            object->matrix);
    }
}

std::string ScaleObjectsCommand::to_string() const
{
    char buffer[128];
    std::snprintf(buffer, 128,
        "Scale objects: pivot = { %10.3f, %10.3f, %10.3f }\n"
        "               scale = { %10.3f, %10.3f, %10.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z);
    return buffer;
}

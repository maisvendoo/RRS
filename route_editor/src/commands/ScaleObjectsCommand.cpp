#include "ScaleObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

ScaleObjectsCommand::ScaleObjectsCommand(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 scale)
    : TransformObjectsCommand(context)
    , pivot(pivot)
    , scale(scale)
{
    update_description();
}

void ScaleObjectsCommand::execute()
{
    for (RouteObject* const object : objects)
    {
        object->scale_relative_to_pivot(pivot, scale, object->matrix);
    }
}

void ScaleObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Scale objects: pivot = { %10.3f, %10.3f, %10.3f }\n"
        "               scale = { %10.3f, %10.3f, %10.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z
    );
}

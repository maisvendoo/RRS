#include "commands/ScaleObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

ScaleObjects::ScaleObjects(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 scale)
    : TransformObjects(context)
    , pivot(pivot)
    , scale(scale)
{
    update_description();
}

void ScaleObjects::execute()
{
    for (const auto& object : objects)
    {
        object->scale_relative_to_pivot(static_cast<vsg::dvec3>(pivot),
            static_cast<vsg::dvec3>(scale), object->matrix);
    }
}

void ScaleObjects::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Scale objects: pivot = { %.3f, %.3f, %.3f }\n"
        "               scale = { %.3f, %.3f, %.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z
    );
}

#include "commands/ScaleObjects.h"

#include "commands/Command.h"
#include "commands/TransformObjects.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

ScaleObjects::ScaleObjects(EditorContext& context, const RouteObjects& objects,
    const vsg::dvec3& pivot, const vsg::dvec3& scale)
    : TransformObjects(context, objects)
    , pivot_(pivot)
    , scale_(scale)
{
    update_description();
}

void ScaleObjects::execute()
{
    for (const auto& object : objects_)
    {
        object->scale_relative_to_pivot(pivot_, scale_, object->matrix);
    }
}

void ScaleObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Scale objects: pivot = { %.3f, %.3f, %.3f }\n"
        "               scale = { %.3f, %.3f, %.3f }",
        pivot_.x, pivot_.y, pivot_.z, scale_.x, scale_.y, scale_.z
    );
}

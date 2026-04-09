#include "commands/TranslateObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

TranslateObjects::TranslateObjects(
    EditorContext& context,
    const vsg::dvec3& translation
)
    : TransformObjects(context, context.selected_objects)
    , translation_(translation)
{
    update_description();
}

void TranslateObjects::execute()
{
    for (const auto& object : objects_)
    {
        object->move(translation_);
    }
}

void TranslateObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Translate objects: { %.3f, %.3f, %.3f }",
        translation_.x, translation_.y, translation_.z
    );
}

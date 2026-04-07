#include "commands/TranslateObjects.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjects.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

TranslateObjects::TranslateObjects(
    EditorContext& context,
    vsg::vec3 translation
)
    : TransformObjects(context)
    , translation(translation)
{
    update_description();
}

void TranslateObjects::execute()
{
    for (const auto& object : objects)
    {
        object->move(translation);
    }
}

void TranslateObjects::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Translate objects: { %.3f, %.3f, %.3f }",
        translation.x, translation.y, translation.z
    );
}

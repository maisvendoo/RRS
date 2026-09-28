#include "editor/commands/TranslateObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"
#include "editor/commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

TranslateObjectsCommand::TranslateObjectsCommand(
    EditorContext& context,
    const RouteObjects& objects,
    const vsg::dvec3& translation
)
    : TransformObjectsCommand(context, objects)
    , translation_(translation)
{
    update_description();
}

void TranslateObjectsCommand::execute()
{
    for (const auto& object : objects_)
    {
        object->move(translation_);
    }
}

void TranslateObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Translate objects: { %.3f, %.3f, %.3f }",
        translation_.x, translation_.y, translation_.z
    );
}

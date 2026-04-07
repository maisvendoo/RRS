#include "commands/MoveObjectsCommand.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "RouteObject.h"
#include "commands/TransformObjectsCommand.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

MoveObjectsCommand::MoveObjectsCommand(
    EditorContext& context,
    vsg::vec3 translation
)
    : TransformObjectsCommand(context)
    , translation(translation)
{
    update_description();
}

void MoveObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->move(translation);
    }
}

void MoveObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Move objects: { %.3f, %.3f, %.3f }",
        translation.x, translation.y, translation.z
    );
}

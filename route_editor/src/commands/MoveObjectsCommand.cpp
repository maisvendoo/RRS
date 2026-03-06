#include "MoveObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>
#include <string>

MoveObjectsCommand::MoveObjectsCommand(
    EditorContext& context,
    vsg::vec3 translation
)
    : Command(context)
    , objects(context.selected_objects)
    , translation(translation)
{
}

void MoveObjectsCommand::execute() const
{
    for (RouteObject* const object : objects)
    {
        object->move(translation);
    }
}

void MoveObjectsCommand::undo() const
{
    for (RouteObject* const object : objects)
    {
        object->move(-translation);
    }
}

std::string MoveObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Move objects: { %10.3f, %10.3f, %10.3f }",
        translation.x, translation.y, translation.z);
    return buffer;
}

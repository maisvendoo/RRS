#include "MoveObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

MoveObjectsCommand::MoveObjectsCommand(
    EditorContext& context,
    vsg::vec3 translation
)
    : Command(context)
    , objects(context.selected_objects)
    , translation(translation)
{
    update_description();
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

void MoveObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Move objects: { %10.3f, %10.3f, %10.3f }",
        translation.x, translation.y, translation.z);
}

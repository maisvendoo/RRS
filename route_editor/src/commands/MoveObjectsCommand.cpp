#include "MoveObjectsCommand.h"

#include "EditorContext.h"
#include "RouteObject.h"
#include "TransformObjectsCommand.h"

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

void MoveObjectsCommand::execute() const
{
    for (RouteObject* const object : objects)
    {
        object->move(translation);
    }
}

void MoveObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Move objects: { %10.3f, %10.3f, %10.3f }",
        translation.x, translation.y, translation.z
    );
}

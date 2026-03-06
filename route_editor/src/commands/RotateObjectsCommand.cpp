#include "RotateObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>

RotateObjectsCommand::RotateObjectsCommand(EditorContext& context,
    vsg::vec3 pivot, vsg::vec3 rotation_deg)
    : Command(context)
    , objects(context.selected_objects)
    , pivot(pivot)
    , rotation_deg(rotation_deg)
{
    update_description();
}

void RotateObjectsCommand::execute() const
{
    for (RouteObject* const object : objects)
    {
        object->rotate_around_pivot(pivot, rotation_deg, object->matrix);
    }
}

void RotateObjectsCommand::undo() const
{
    for (RouteObject* const object : objects)
    {
        object->rotate_around_pivot(pivot, -rotation_deg, object->matrix);
    }
}

void RotateObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Rotate objects: pivot = { %10.3f, %10.3f, %10.3f }\n"
        "                rotation_deg = { %10.3f, %10.3f, %10.3f }",
        pivot.x, pivot.y, pivot.z,
        rotation_deg.x, rotation_deg.y, rotation_deg.z);
}

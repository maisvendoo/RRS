#include "MoveObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <vector>

MoveObjectsCommand::MoveObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects,
    vsg::vec3 translation
)
    : objects(objects)
    , translation(translation)
{
}

void MoveObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->move(translation);
    }
}

void MoveObjectsCommand::undo()
{
    for (const auto& object : objects)
    {
        object->move(-translation);
    }
}

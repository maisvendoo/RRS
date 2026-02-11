#include "ScaleObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

ScaleObjectsCommand::ScaleObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects,
    vsg::vec3 scale
)
    : objects(objects)
    , scale(scale)
{
}

void ScaleObjectsCommand::execute() const
{
    // TODO
}

void ScaleObjectsCommand::undo() const
{
    // TODO
}

std::string ScaleObjectsCommand::to_string() const
{
    // TODO
}

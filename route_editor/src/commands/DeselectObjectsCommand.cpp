#include "DeselectObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

DeselectObjectsCommand::DeselectObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects)
    : objects(objects)
{
}

void DeselectObjectsCommand::execute() const
{
    // TODO
}

void DeselectObjectsCommand::undo() const
{
    // TODO
}

std::string DeselectObjectsCommand::to_string() const
{
    // TODO
}

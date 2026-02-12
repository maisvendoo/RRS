#include "ShowObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

ShowObjectsCommand::ShowObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects)
    : objects(objects)
{
}

void ShowObjectsCommand::execute() const
{
    // TODO
}

void ShowObjectsCommand::undo() const
{
    // TODO
}

std::string ShowObjectsCommand::to_string() const
{
    // TODO
}

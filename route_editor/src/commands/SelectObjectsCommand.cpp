#include "SelectObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

SelectObjectsCommand::SelectObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects)
    : objects(objects)
{
}

void SelectObjectsCommand::execute() const
{
    // TODO
}

void SelectObjectsCommand::undo() const
{
    // TODO
}

std::string SelectObjectsCommand::to_string() const
{
    // TODO
}

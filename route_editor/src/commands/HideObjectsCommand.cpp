#include "HideObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

HideObjectsCommand::HideObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects)
    : objects(objects)
{
}

void HideObjectsCommand::execute() const
{
    // TODO
}

void HideObjectsCommand::undo() const
{
    // TODO
}

std::string HideObjectsCommand::to_string() const
{
    // TODO
}

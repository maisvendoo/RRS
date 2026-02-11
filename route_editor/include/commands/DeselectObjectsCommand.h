#ifndef DESELECT_OBJECTS_COMMAND_H
#define DESELECT_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

class RouteObject;

class DeselectObjectsCommand : public Command
{
public:
    DeselectObjectsCommand(const std::vector<vsg::ref_ptr<RouteObject>>& objects);

    virtual ~DeselectObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
};


#endif // DESELECT_OBJECTS_COMMAND_H

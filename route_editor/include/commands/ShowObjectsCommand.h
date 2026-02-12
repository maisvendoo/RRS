#ifndef SHOW_OBJECTS_COMMAND_H
#define SHOW_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

class RouteObject;

class ShowObjectsCommand : public Command
{
public:
    ShowObjectsCommand(const std::vector<vsg::ref_ptr<RouteObject>>& objects);

    virtual ~ShowObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
};


#endif // SHOW_OBJECTS_COMMAND_H

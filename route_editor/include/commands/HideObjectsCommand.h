#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

class RouteObject;

class HideObjectsCommand : public Command
{
public:
    HideObjectsCommand(const std::vector<vsg::ref_ptr<RouteObject>>& objects);

    virtual ~HideObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
};


#endif // HIDE_OBJECTS_COMMAND_H

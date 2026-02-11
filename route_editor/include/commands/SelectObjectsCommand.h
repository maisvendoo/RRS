#ifndef SELECT_OBJECTS_COMMAND_H
#define SELECT_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>

#include <string>
#include <vector>

class RouteObject;

class SelectObjectsCommand : public Command
{
public:
    SelectObjectsCommand(const std::vector<vsg::ref_ptr<RouteObject>>& objects);

    virtual ~SelectObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
};


#endif // SELECT_OBJECTS_COMMAND_H

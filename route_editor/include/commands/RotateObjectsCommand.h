#ifndef ROTATE_OBJECTS_COMMAND_H
#define ROTATE_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

class RouteObject;

class RotateObjectsCommand : public Command
{
public:
    RotateObjectsCommand(
        const std::vector<vsg::ref_ptr<RouteObject>>& objects,
        vsg::vec3 rotation_deg
    );

    virtual ~RotateObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
    vsg::vec3 rotation_deg;
};


#endif // SELECT_OBJECTS_COMMAND_H

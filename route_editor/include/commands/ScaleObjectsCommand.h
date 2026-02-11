#ifndef SCALE_OBJECTS_COMMAND_H
#define SCALE_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

class RouteObject;

class ScaleObjectsCommand : public Command
{
public:
    ScaleObjectsCommand(
        const std::vector<vsg::ref_ptr<RouteObject>>& objects,
        vsg::vec3 scale
    );

    virtual ~ScaleObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const std::vector<vsg::ref_ptr<RouteObject>> objects;
    vsg::vec3 scale;
};


#endif // SCALE_OBJECTS_COMMAND_H

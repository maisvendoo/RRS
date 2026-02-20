#ifndef SCALE_OBJECTS_COMMAND_H
#define SCALE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <string>

class ScaleObjectsCommand : public Command
{
public:
    ScaleObjectsCommand(const RouteObjects& objects, vsg::vec3 pivot,
        vsg::vec3 scale);

    virtual ~ScaleObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects;
    vsg::vec3 pivot;
    vsg::vec3 scale;
};

#endif // SCALE_OBJECTS_COMMAND_H

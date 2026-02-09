#ifndef MOVE_OBJECTS_COMMAND_H
#define MOVE_OBJECTS_COMMAND_H

#include "Command.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <vector>

class RouteObject;

class MoveObjectsCommand : public Command
{
public:
    MoveObjectsCommand(
        const std::vector<vsg::ref_ptr<RouteObject>>& objects,
        vsg::vec3 translation
    );

    virtual ~MoveObjectsCommand() override = default;
    virtual void execute() override;
    virtual void undo() override;

private:
    std::vector<vsg::ref_ptr<RouteObject>> objects;
    vsg::vec3 translation;
};

#endif // MOVE_OBJECTS_COMMAND_H

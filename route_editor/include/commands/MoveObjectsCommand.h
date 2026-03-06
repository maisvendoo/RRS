#ifndef MOVE_OBJECTS_COMMAND_H
#define MOVE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <string>

struct EditorContext;

class MoveObjectsCommand : public Command
{
public:
    MoveObjectsCommand(EditorContext& context, vsg::vec3 translation);
    virtual ~MoveObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects;
    vsg::vec3 translation;
};

#endif // MOVE_OBJECTS_COMMAND_H

#ifndef MOVE_OBJECTS_COMMAND_H
#define MOVE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObjects.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class MoveObjectsCommand : public Command
{
public:
    MoveObjectsCommand(EditorContext& context, vsg::vec3 translation);
    virtual ~MoveObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual void update_description() override;

private:
    const RouteObjects objects;
    vsg::vec3 translation;
};

#endif // MOVE_OBJECTS_COMMAND_H

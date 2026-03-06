#ifndef ROTATE_OBJECTS_COMMAND_H
#define ROTATE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class RotateObjectsCommand : public Command
{
public:
    RotateObjectsCommand(EditorContext& context,vsg::vec3 pivot,
        vsg::vec3 rotation_deg);

    virtual ~RotateObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual void update_description() override;

private:
    const RouteObjects objects;
    vsg::vec3 pivot;
    vsg::vec3 rotation_deg;
};

#endif // ROTATE_OBJECTS_COMMAND_H

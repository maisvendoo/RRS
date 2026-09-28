#ifndef EDITOR_COMMANDS_ADD_OBJECT_COMMAND_H
#define EDITOR_COMMANDS_ADD_OBJECT_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class RouteObject;

class AddObjectCommand : public Command
{
public:
    AddObjectCommand(EditorContext& editor_context,
        const vsg::ref_ptr<RouteObject>& object);

    virtual ~AddObjectCommand() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const vsg::ref_ptr<RouteObject> object_to_add_;
    const RouteObjects objects_to_deselect_;
};

#endif // EDITOR_COMMANDS_ADD_OBJECT_COMMAND_H

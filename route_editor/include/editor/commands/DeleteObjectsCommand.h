#ifndef EDITOR_COMMANDS_DELETE_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_DELETE_OBJECTS_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

struct EditorContext;

class DeleteObjectsCommand : public Command
{
public:
    explicit DeleteObjectsCommand(EditorContext& editor_context);

    virtual ~DeleteObjectsCommand() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const RouteObjects objects_;
};

#endif // EDITOR_COMMANDS_DELETE_OBJECTS_COMMAND_H

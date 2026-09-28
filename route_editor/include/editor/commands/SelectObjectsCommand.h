#ifndef EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

struct EditorContext;

class SelectObjectsCommand : public Command
{
public:
    explicit SelectObjectsCommand(EditorContext& context);

    virtual ~SelectObjectsCommand() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

public:
    RouteObjects objects_to_select;
    RouteObjects objects_to_deselect;
};

#endif // EDITOR_COMMANDS_SELECT_OBJECTS_COMMAND_H

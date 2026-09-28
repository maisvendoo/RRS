#ifndef EDITOR_COMMANDS_PASTE_OBJECTS_COMMAND_H
#define EDITOR_COMMANDS_PASTE_OBJECTS_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

struct EditorContext;

class PasteObjectsCommand : public Command
{
public:
    explicit PasteObjectsCommand(EditorContext& context);

    virtual ~PasteObjectsCommand() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const RouteObjects objects_to_paste_;
    RouteObjects pasted_objects_;
    const RouteObjects objects_to_deselect_;
};

#endif // EDITOR_COMMANDS_PASTE_OBJECTS_COMMAND_H

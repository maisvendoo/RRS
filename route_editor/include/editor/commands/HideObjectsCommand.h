#ifndef EDITOR_COMMAND_HIDE_OBJECTS_COMMAND_H
#define EDITOR_COMMAND_HIDE_OBJECTS_COMMAND_H

#include "editor/RouteObjects.h"
#include "editor/commands/Command.h"

struct EditorContext;

class HideObjectsCommand : public Command
{
public:
    explicit HideObjectsCommand(EditorContext& context);

    virtual ~HideObjectsCommand() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

public:
    const RouteObjects objects_to_hide;
    const RouteObjects objects_to_show;
};


#endif // EDITOR_COMMAND_HIDE_OBJECTS_COMMAND_H

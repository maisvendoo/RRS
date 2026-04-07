#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "commands/Command.h"
#include "RouteObject.h"

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


#endif // HIDE_OBJECTS_COMMAND_H

#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObjects.h"

struct EditorContext;

class HideObjectsCommand : public Command
{
public:
    explicit HideObjectsCommand(EditorContext& context);
    virtual ~HideObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual void update_description() override;

public:
    RouteObjects objects_to_hide;
    RouteObjects objects_to_show;
};


#endif // HIDE_OBJECTS_COMMAND_H

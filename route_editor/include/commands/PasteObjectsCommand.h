#ifndef PASTE_OBJECTS_COMMAND_H
#define PASTE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

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
    const RouteObjects objects_to_paste;
    RouteObjects pasted_objects;
    const RouteObjects objects_to_deselect;
};

#endif // PASTE_OBJECTS_COMMAND_H

#ifndef PASTE_OBJECTS_H
#define PASTE_OBJECTS_H

#include "commands/Command.h"
#include "RouteObject.h"

struct EditorContext;

class PasteObjects : public Command
{
public:
    explicit PasteObjects(EditorContext& context);
    virtual ~PasteObjects() override = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    const RouteObjects objects_to_paste_;
    RouteObjects pasted_objects_;
    const RouteObjects objects_to_deselect_;
};

#endif // PASTE_OBJECTS_H

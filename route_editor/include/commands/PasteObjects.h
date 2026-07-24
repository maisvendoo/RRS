#ifndef PASTE_OBJECTS_H
#define PASTE_OBJECTS_H

#include "RouteObject.h"
#include "commands/Command.h"

struct EditorContext;
class Route;

class PasteObjects : public Command
{
public:
    PasteObjects(
        EditorContext& context,
        const vsg::ref_ptr<Route>& route
    );

    virtual ~PasteObjects() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const RouteObjects objects_to_paste_;
    RouteObjects pasted_objects_;
    const RouteObjects objects_to_deselect_;
    const vsg::ref_ptr<Route>& route;
};

#endif // PASTE_OBJECTS_H

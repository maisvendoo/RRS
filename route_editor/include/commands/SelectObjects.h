#ifndef SELECT_OBJECTS_H
#define SELECT_OBJECTS_H

#include "RouteObjects.h"
#include "commands/Command.h"

struct EditorContext;
class Gizmo;

class SelectObjects : public Command
{
public:
    SelectObjects(
        EditorContext& context,
        const vsg::ref_ptr<Gizmo>& gizmo
    );

    virtual ~SelectObjects() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

public:
    RouteObjects objects_to_select;
    RouteObjects objects_to_deselect;

private:
    const vsg::ref_ptr<Gizmo>& gizmo;
};

#endif // SELECT_OBJECTS_H

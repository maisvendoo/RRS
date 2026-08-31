#ifndef DELETE_OBJECTS_H
#define DELETE_OBJECTS_H

#include "RouteObjects.h"
#include "commands/Command.h"

struct EditorContext;
class Gizmo;
class Route;

class DeleteObjects : public Command
{
public:
    DeleteObjects(
        EditorContext& context,
        const vsg::ref_ptr<Route>& route,
        const vsg::ref_ptr<Gizmo>& gizmo
    );

    virtual ~DeleteObjects() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const RouteObjects objects_;
    const vsg::ref_ptr<Route>& route;
    const vsg::ref_ptr<Gizmo>& gizmo;
};

#endif // DELETE_OBJECTS_H

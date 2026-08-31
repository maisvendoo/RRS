#ifndef ADD_OBJECT_H
#define ADD_OBJECT_H

#include "RouteObjects.h"
#include "commands/Command.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class Gizmo;
class Route;

namespace vsg
{

class RouteObject;

}

class AddObject : public Command
{
public:
    AddObject(
        EditorContext& context,
        vsg::ref_ptr<RouteObject> object,
        const vsg::ref_ptr<Route>& route,
        const vsg::ref_ptr<Gizmo>& gizmo
    );

    virtual ~AddObject() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

private:
    const vsg::ref_ptr<RouteObject> object_to_add_;
    const RouteObjects objects_to_deselect_;
    const vsg::ref_ptr<Route>& route;
    const vsg::ref_ptr<Gizmo>& gizmo;
};

#endif // ADD_OBJECT_H

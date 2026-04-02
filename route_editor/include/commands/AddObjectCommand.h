#ifndef ADD_OBJECT_COMMAND_H
#define ADD_OBJECT_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;

namespace vsg
{

class RouteObject;

}

class AddObjectCommand : public Command
{
public:
    AddObjectCommand(EditorContext& context, vsg::ref_ptr<RouteObject> object);
    virtual ~AddObjectCommand() override = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    const vsg::ref_ptr<RouteObject> object_to_add;
    const RouteObjects objects_to_deselect;
};

#endif // ADD_OBJECT_COMMAND_H

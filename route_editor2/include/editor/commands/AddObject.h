#ifndef EDITOR_ADD_OBJECT_H
#define EDITOR_ADD_OBJECT_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class RouteObject;

/// Добавление объекта в маршрут
class AddObject : public Command
{
public:
    AddObject(EditorContext& context,
        vsg::ref_ptr<RouteObject> object);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    vsg::ref_ptr<RouteObject> object_to_add_;
    RouteObjects objects_to_deselect_;
};

#endif // EDITOR_ADD_OBJECT_H

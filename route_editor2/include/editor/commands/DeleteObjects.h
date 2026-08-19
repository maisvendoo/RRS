#ifndef EDITOR_DELETE_OBJECTS_H
#define EDITOR_DELETE_OBJECTS_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

struct EditorContext;

/// Удаление выделенных объектов
class DeleteObjects : public Command
{
public:
    explicit DeleteObjects(EditorContext& context);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    RouteObjects objects_;
};

#endif // EDITOR_DELETE_OBJECTS_H

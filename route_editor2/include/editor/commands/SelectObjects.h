#ifndef EDITOR_SELECT_OBJECTS_H
#define EDITOR_SELECT_OBJECTS_H

#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

struct EditorContext;

/// Выделение/снятие выделения объектов
class SelectObjects : public Command
{
public:
    explicit SelectObjects(EditorContext& context);

    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

public:
    RouteObjects objects_to_select;
    RouteObjects objects_to_deselect;
};

#endif // EDITOR_SELECT_OBJECTS_H

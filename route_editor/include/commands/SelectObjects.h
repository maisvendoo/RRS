#ifndef SELECT_OBJECTS_H
#define SELECT_OBJECTS_H

#include "commands/Command.h"
#include "RouteObject.h"

struct EditorContext;

class SelectObjects : public Command
{
public:
    explicit SelectObjects(EditorContext& context);
    virtual ~SelectObjects() override = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

public:
    RouteObjects objects_to_select;
    RouteObjects objects_to_deselect;
};

#endif // SELECT_OBJECTS_H

#ifndef SELECT_OBJECTS_COMMAND_H
#define SELECT_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

class Gizmo;

class SelectObjectsCommand : public Command
{
public:
    explicit SelectObjectsCommand(EditorContext& context);
    virtual ~SelectObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

public:
    RouteObjects objects_to_select;
    RouteObjects objects_to_deselect;
};


#endif // SELECT_OBJECTS_COMMAND_H

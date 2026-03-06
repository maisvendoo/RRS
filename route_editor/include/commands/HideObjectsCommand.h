#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

struct EditorContext;

class HideObjectsCommand : public Command
{
public:
    explicit HideObjectsCommand(EditorContext& context);
    virtual ~HideObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

public:
    RouteObjects objects_to_hide;
    RouteObjects objects_to_show;
};


#endif // HIDE_OBJECTS_COMMAND_H

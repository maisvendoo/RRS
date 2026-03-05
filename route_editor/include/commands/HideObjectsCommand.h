#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "Command.h"
#include "EditorContext.h"
#include "RouteObject.h"

#include <string>

class HideObjectsCommand : public Command
{
public:
    HideObjectsCommand(
        EditorContext& context,
        const RouteObjects& objects_to_hide,
        const RouteObjects& objects_to_show
    );

    HideObjectsCommand(
        EditorContext& context,
        const RouteObjects& objects_to_hide,
        const RouteObjects&& objects_to_show
    );

    HideObjectsCommand(
        EditorContext& context,
        const RouteObjects&& objects_to_hide,
        const RouteObjects& objects_to_show
    );

    HideObjectsCommand(
        EditorContext& context,
        const RouteObjects&& objects_to_hide,
        const RouteObjects&& objects_to_show
    );

    virtual ~HideObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects_to_hide;
    const RouteObjects objects_to_show;
};


#endif // HIDE_OBJECTS_COMMAND_H

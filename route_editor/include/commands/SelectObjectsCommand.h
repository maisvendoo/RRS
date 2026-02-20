#ifndef SELECT_OBJECTS_COMMAND_H
#define SELECT_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

class Gizmo;

class SelectObjectsCommand : public Command
{
public:
    SelectObjectsCommand(
        const RouteObjects& objects_to_select,
        const RouteObjects& objects_to_deselect
    );

    SelectObjectsCommand(
        const RouteObjects& objects_to_select,
        const RouteObjects&& objects_to_deselect
    );

    SelectObjectsCommand(
        const RouteObjects&& objects_to_select,
        const RouteObjects& objects_to_deselect
    );

    SelectObjectsCommand(
        const RouteObjects&& objects_to_select,
        const RouteObjects&& objects_to_deselect
    );

    virtual ~SelectObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

    static void set_gizmo(Gizmo* gizmo);

private:
    static Gizmo* s_gizmo;

    const RouteObjects objects_to_select;
    const RouteObjects objects_to_deselect;
};


#endif // SELECT_OBJECTS_COMMAND_H

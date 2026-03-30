#ifndef DELETE_OBJECTS_COMMAND_H
#define DELETE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObjects.h"

struct EditorContext;

class DeleteObjectsCommand : public Command
{
public:
    explicit DeleteObjectsCommand(EditorContext& context);
    virtual ~DeleteObjectsCommand() override = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    const RouteObjects objects;
};

#endif // DELETE_OBJECTS_COMMAND_H

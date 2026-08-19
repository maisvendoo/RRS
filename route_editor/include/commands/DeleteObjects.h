#ifndef DELETE_OBJECTS_H
#define DELETE_OBJECTS_H

#include "commands/Command.h"
#include "RouteObject.h"

struct EditorContext;

class DeleteObjects : public Command
{
public:
    explicit DeleteObjects(EditorContext& context);
    virtual ~DeleteObjects() override = default;
    virtual void execute() override;
    virtual void undo() override;
    virtual void update_description() override;

private:
    const RouteObjects objects_;
};

#endif // DELETE_OBJECTS_H

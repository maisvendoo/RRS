#ifndef HIDE_OBJECTS_H
#define HIDE_OBJECTS_H

#include "RouteObjects.h"
#include "commands/Command.h"

struct EditorContext;

class HideObjects : public Command
{
public:
    explicit HideObjects(EditorContext& context);

    virtual ~HideObjects() override = default;

    virtual void execute() override;

    virtual void undo() override;

    virtual void update_description() override;

public:
    const RouteObjects objects_to_hide;
    const RouteObjects objects_to_show;
};


#endif // HIDE_OBJECTS_H

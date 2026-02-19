#ifndef DESELECT_OBJECTS_COMMAND_H
#define DESELECT_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

class DeselectObjectsCommand : public Command
{
public:
    DeselectObjectsCommand(const RouteObjects& objects);
    virtual ~DeselectObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects;
};


#endif // DESELECT_OBJECTS_COMMAND_H

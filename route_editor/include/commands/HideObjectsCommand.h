#ifndef HIDE_OBJECTS_COMMAND_H
#define HIDE_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

class HideObjectsCommand : public Command
{
public:
    HideObjectsCommand(const RouteObjects& objects);
    virtual ~HideObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects;
};


#endif // HIDE_OBJECTS_COMMAND_H

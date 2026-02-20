#ifndef SHOW_OBJECTS_COMMAND_H
#define SHOW_OBJECTS_COMMAND_H

#include "Command.h"
#include "RouteObject.h"

#include <string>

class ShowObjectsCommand : public Command
{
public:
    ShowObjectsCommand(const RouteObjects& objects);
    virtual ~ShowObjectsCommand() override = default;
    virtual void execute() const override;
    virtual void undo() const override;
    virtual std::string to_string() const override;

private:
    const RouteObjects objects;
};


#endif // SHOW_OBJECTS_COMMAND_H

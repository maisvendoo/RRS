#ifndef COMMAND_H
#define COMMAND_H

#include <string>

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute() const = 0;
    virtual void undo() const = 0;
    virtual std::string to_string() const = 0;
};

#endif // COMMAND_H

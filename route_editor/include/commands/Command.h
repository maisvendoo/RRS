#ifndef COMMAND_H
#define COMMAND_H

#include <string>

struct EditorContext;

class Command
{
public:
    explicit Command(EditorContext& context) : context(context) {}
    virtual ~Command() = default;
    virtual void execute() const = 0;
    virtual void undo() const = 0;
    virtual std::string to_string() const = 0;

protected:
    EditorContext& context;
};

#endif // COMMAND_H

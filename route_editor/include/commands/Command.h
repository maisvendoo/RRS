#ifndef COMMAND_H
#define COMMAND_H

#define COMMAND_DESCRIPTION_BUFFER_SIZE 256

struct EditorContext;

class Command
{
public:
    explicit Command(EditorContext& context) : context(context) {}
    virtual ~Command() = default;
    virtual void execute() const = 0;
    virtual void undo() const = 0;
    virtual void update_description() = 0;
    const char* get_description() const { return description; }

protected:
    EditorContext& context;
    char description[COMMAND_DESCRIPTION_BUFFER_SIZE];
};

#endif // COMMAND_H

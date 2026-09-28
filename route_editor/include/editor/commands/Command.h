#ifndef EDITOR_COMMANDS_COMMAND_H
#define EDITOR_COMMANDS_COMMAND_H

#define COMMAND_DESCRIPTION_BUFFER_SIZE 256

struct EditorContext;

class Command
{
public:
    explicit Command(EditorContext& context);
    virtual ~Command();

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void update_description() = 0;

    const char* get_description() const;

protected:
    EditorContext& editor_context;
    char description[COMMAND_DESCRIPTION_BUFFER_SIZE];
};

#endif // EDITOR_COMMANDS_COMMAND_H

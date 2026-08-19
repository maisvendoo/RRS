#ifndef COMMAND_H
#define COMMAND_H

#define COMMAND_DESCRIPTION_BUFFER_SIZE 256

struct EditorContext;

class Command
{
public:
    explicit Command(EditorContext& context) : context_(context) {}
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void update_description() = 0;
    const char* get_description() const { return description_; }

protected:
    EditorContext& context_;
    char description_[COMMAND_DESCRIPTION_BUFFER_SIZE];
};

#endif // COMMAND_H

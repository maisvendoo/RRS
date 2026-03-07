#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <cstddef>

class Command;

class CommandList
{
private:
    struct CommandNode
    {
        ~CommandNode();

        const Command* command = nullptr;
        CommandNode* prev = nullptr;
        CommandNode* next = nullptr;
    };

public:
    ~CommandList();

    void push(const Command* command, bool execute);
    void undo();
    void redo();

    const CommandNode* get_active() const;
    const CommandNode* get_tail() const;

private:
    void push_(const Command* command, bool execute);

private:
    CommandNode* tail = nullptr;
    CommandNode* active = nullptr;
    std::size_t size = 0;
};

#endif // COMMAND_LIST_H

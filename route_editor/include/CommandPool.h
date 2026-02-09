#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include <cstddef>

class Command;

class CommandPool
{
public:
    ~CommandPool();

    void push(const Command* command);
    void undo();
    void redo();

private:
    struct CommandNode
    {
        const Command* command;
        CommandNode* next;
    };

    CommandNode* root = nullptr;
    std::size_t size = 0;
};

#endif // COMMAND_POOL_H

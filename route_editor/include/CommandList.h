#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <cstddef>

#include <string>

class Command;

class CommandList
{
private:
    struct CommandNode
    {
        ~CommandNode();

        std::string to_string();

        const Command* command = nullptr;
        CommandNode* prev = nullptr;
        CommandNode* next = nullptr;
    };

public:
    ~CommandList();

    void push(const Command* command);
    void undo();
    void redo();

    void print();

private:
    CommandNode* tail = nullptr;
    std::size_t size = 0;

    CommandNode* active = nullptr;
    std::size_t active_size = 0;
};

#endif // COMMAND_LIST_H

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

    CommandNode* get_active() const
    {
        return active;
    }

    CommandNode* get_tail() const
    {
        return tail;
    }

private:
    CommandNode* tail = nullptr;
    CommandNode* active = nullptr;
    std::size_t size = 0;
};

#endif // COMMAND_LIST_H

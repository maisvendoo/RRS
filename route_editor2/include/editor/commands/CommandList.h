#ifndef EDITOR_COMMAND_LIST_H
#define EDITOR_COMMAND_LIST_H

#include <cstddef>

class Command;

/// Двусвязный список выполненных команд с ограничением размера
class CommandList
{
private:
    struct CommandNode
    {
        ~CommandNode();

        Command* command = nullptr;
        CommandNode* prev = nullptr;
        CommandNode* next = nullptr;
    };

public:
    ~CommandList();

    /// Добавить команду в список; execute == true — выполнить сразу
    void push(Command* command, bool execute);
    void undo();
    void redo();

    const CommandNode* get_active() const;
    const CommandNode* get_tail() const;

private:
    void push_(Command* command, bool execute);

private:
    CommandNode* tail_ = nullptr;
    CommandNode* active_ = nullptr;
    std::size_t size_ = 0;
};

#endif // EDITOR_COMMAND_LIST_H

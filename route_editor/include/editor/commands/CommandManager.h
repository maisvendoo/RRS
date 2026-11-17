#ifndef EDITOR_COMMANDS_COMMAND_MANAGER_H
#define EDITOR_COMMANDS_COMMAND_MANAGER_H

#include <deque>
#include <functional>
#include <memory>

class Command;

class CommandManager
{
private:
    using Func = std::function<void(const std::unique_ptr<Command>&)>;

public:
    CommandManager();
    ~CommandManager();

    void push(std::unique_ptr<Command>&& command);
    void undo();
    void redo();

    void for_each_command(const Func& func);
    void for_each_undone(const Func& func);

private:
    std::deque<std::unique_ptr<Command>> commands;
    std::deque<std::unique_ptr<Command>> undone_commands;
};

#endif // EDITOR_COMMANDS_COMMAND_MANAGER_H

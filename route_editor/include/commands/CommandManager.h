#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <deque>
#include <functional>
#include <memory>

class Command;

class CommandManager
{
public:
    CommandManager();
    ~CommandManager();

    void push(std::unique_ptr<Command>&& command);
    void undo();
    void redo();

    void for_each_command(
        const std::function<void(const std::unique_ptr<Command>&)>& func);

    void for_each_undone(
        const std::function<void(const std::unique_ptr<Command>&)>& func);

private:
    std::deque<std::unique_ptr<Command>> commands;
    std::deque<std::unique_ptr<Command>> undone_commands;
};

#endif // COMMAND_MANAGER_H

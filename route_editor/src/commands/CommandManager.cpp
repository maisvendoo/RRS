#include "commands/CommandManager.h"

#include "commands/Command.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

static constexpr std::size_t MAX_COMMAND_COUNT = 50;

CommandManager::CommandManager() = default;

CommandManager::~CommandManager() = default;

void CommandManager::push(std::unique_ptr<Command>&& command)
{
    undone_commands.clear();

    while (commands.size() >= MAX_COMMAND_COUNT)
    {
        commands.pop_front();
    }

    commands.push_back(std::move(command));
}

void CommandManager::undo()
{
    if (commands.empty())
    {
        return;
    }

    auto command = std::move(commands.back());
    commands.pop_back();
    command->undo();
    undone_commands.push_front(std::move(command));
}

void CommandManager::redo()
{
    if (undone_commands.empty())
    {
        return;
    }

    auto command = std::move(undone_commands.front());
    undone_commands.pop_front();
    command->execute();
    commands.push_back(std::move(command));
}

void CommandManager::for_each_command(
    const std::function<void(const std::unique_ptr<Command>&)>& func)
{
    std::for_each(commands.begin(), commands.end(), func);
}

void CommandManager::for_each_undone(
    const std::function<void(const std::unique_ptr<Command>&)>& func)
{
    std::for_each(undone_commands.begin(), undone_commands.end(), func);
}

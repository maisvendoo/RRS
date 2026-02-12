#include "CommandList.h"

#include "Command.h"

#include <cstddef>
#include <cstdio>
#include <string>

static constexpr std::size_t MAX_SAVED_COMMANDS = 6;

CommandList::CommandNode::~CommandNode()
{
    delete command;
}

std::string CommandList::CommandNode::to_string()
{
    char buffer[512];
    std::snprintf(buffer, 512, "command: %s\nnext: %p\nprev: %p\n",
        command->to_string().c_str(), (void*)next, (void*)prev);
    return command->to_string().c_str();
}

CommandList::~CommandList()
{
    // Delete all command nodes from tail
    CommandNode* curr = tail;
    while (curr)
    {
        CommandNode* const prev = curr->prev;
        delete curr;
        curr = prev;
    }
}

void CommandList::push(const Command* command)
{
    // If active node is not tail node (after undos)
    if (active != tail)
    {
        // Delete all command nodes from tail to active (exclusive)
        CommandNode* curr = tail;
        while (curr != active)
        {
            CommandNode* const prev = curr->prev;
            delete curr;
            curr = prev;
            --size;
        }
        // Now tail is equal to active
        tail = active;
        tail->next = nullptr;
    }

    if (size == 0)
    {
        tail = active = new CommandNode{command, nullptr, nullptr};
        command->execute();
        size = 1;
        return;
    }

    if (size == MAX_SAVED_COMMANDS)
    {
        CommandNode* head = tail;
        while (head->prev)
        {
            head = head->prev;
        }
        CommandNode* const next = head->next;
        if (active == head)
        {
            active = next;
        }
        delete head;
        next->prev = nullptr;
        --size;
    }

    CommandNode* new_node = new CommandNode{command, tail, nullptr};
    command->execute();
    tail->next = new_node;
    tail = active = new_node;
    ++size;
}

void CommandList::undo()
{
    if (!active)
    {
        return;
    }

    active->command->undo();
    active = active->prev;
}

void CommandList::redo()
{
    if (active == tail)
    {
        return;
    }

    active = active->next;
    active->command->execute();
}

void CommandList::print()
{
    for (int i = 0; i < 80; ++i)
    {
        std::printf("-");
    }
    std::printf("\n");
    std::printf("Command list:\n");
    std::printf("size: %zu\n", size);

    CommandNode* curr = tail;
    while (curr)
    {
        if (curr == active)
        {
            std::printf("ACTIVE ");
        }
        std::printf("%p %s\n", (void*)curr, curr->to_string().c_str());
        curr = curr->prev;
    }
}

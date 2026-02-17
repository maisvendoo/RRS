#include "CommandList.h"

#include "Command.h"

#include <cstddef>

static constexpr std::size_t MAX_SAVED_COMMANDS = 5;

CommandList::CommandNode::~CommandNode()
{
    delete command;
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

void CommandList::push(const Command* command, bool execute)
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
        push_inner(command, execute);
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

    push_inner(command, execute);
}

void CommandList::undo()
{
    if (active)
    {
        active->command->undo();
        active = active->prev;
    }
}

void CommandList::redo()
{
    if (active != tail)
    {
        active = active->next;
        active->command->execute();
    }
}

const CommandList::CommandNode* CommandList::get_tail() const
{
    return tail;
}

const CommandList::CommandNode* CommandList::get_active() const
{
    return active;
}

void CommandList::push_inner(const Command* command, bool execute)
{
    CommandNode* const new_node = new CommandNode{command, tail, nullptr};

    if (tail)
    {
        tail->next = new_node;
    }

    tail = active = new_node;
    ++size;

    if (execute)
    {
        command->execute();
    }
}

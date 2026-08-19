#include "commands/CommandList.h"

#include "commands/Command.h"

#include <cstddef>

static constexpr std::size_t MAX_SAVED_COMMANDS = 50;

CommandList::CommandNode::~CommandNode()
{
    delete command;
}

CommandList::~CommandList()
{
    // Delete all command nodes starting from tail_
    CommandNode* curr = tail_;
    while (curr)
    {
        CommandNode* const prev = curr->prev;
        delete curr;
        curr = prev;
    }
}

void CommandList::push(Command* command, bool execute)
{
    // If active_ node is not tail_ node (after undos)
    if (active_ != tail_)
    {
        // Delete all command nodes from tail_ to active_ (exclusive)
        CommandNode* curr = tail_;
        while (curr != active_)
        {
            CommandNode* const prev = curr->prev;
            delete curr;
            curr = prev;
            --size_;
        }

        // Now tail_ is equal to active_
        tail_ = active_;

        if (tail_)
        {
            tail_->next = nullptr;
        }
    }

    if (size_ == 0)
    {
        push_(command, execute);
        return;
    }

    if (size_ == MAX_SAVED_COMMANDS)
    {
        // Find head of list (first node)
        CommandNode* head = tail_;
        while (head->prev)
        {
            head = head->prev;
        }

        // Move head pointer to next node and decrease list size
        // (first node becomes deleted)
        CommandNode* const next = head->next;
        if (active_ == head)
        {
            active_ = next;
        }
        delete head;
        next->prev = nullptr;
        --size_;
    }

    push_(command, execute);
}

void CommandList::undo()
{
    if (active_)
    {
        active_->command->undo();
        active_ = active_->prev;
    }
}

void CommandList::redo()
{
    if (active_ == tail_)
    {
        return;
    }

    if (active_)
    {
        active_ = active_->next;
        active_->command->execute();
        return;
    }

    CommandNode* head = tail_;
    while (head->prev)
    {
        head = head->prev;
    }
    active_ = head;
    active_->command->execute();
}

const CommandList::CommandNode* CommandList::get_tail() const
{
    return tail_;
}

const CommandList::CommandNode* CommandList::get_active() const
{
    return active_;
}

void CommandList::push_(Command* command, bool execute)
{
    CommandNode* const new_node = new CommandNode{command, tail_, nullptr};

    if (tail_)
    {
        tail_->next = new_node;
    }

    tail_ = active_ = new_node;
    ++size_;

    if (execute)
    {
        command->execute();
    }
}

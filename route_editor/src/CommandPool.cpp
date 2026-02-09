#include "CommandPool.h"

CommandPool::~CommandPool()
{

}

void CommandPool::push(const Command* command)
{
    CommandNode* const * curr = &root;
    while (*curr)
    {
        curr = &(*curr)->next;
    }
}

void CommandPool::undo()
{

}

void CommandPool::redo()
{

}


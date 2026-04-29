#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <list>
#include <memory>

class Command;

class CommandList
{
public:


private:
    std::list<std::unique_ptr<Command>> commands_;
};

#endif // COMMAND_LIST_H

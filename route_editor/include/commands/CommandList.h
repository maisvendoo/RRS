#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include <cstddef>

class Command;

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

#endif // COMMAND_LIST_H


/*

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <memory>
#include <stack>
#include <string>
#include <utility>

static constexpr std::size_t MAX_COMMAND_COUNT = 50;

class Command
{
public:
    Command() {
        printf("Construct Command\n");
    }
    virtual ~Command() {
        printf("Destruct Command\n");
    }
    virtual void execute() {}
    virtual void undo() {}
};

class CommandFirst : public Command
{
public:
    CommandFirst() {
        printf("Construct CommandFirst\n");
    }
    virtual ~CommandFirst() override {
        printf("Destruct CommandFirst\n");
    }
    int a;
    virtual void execute() override {
        printf("%d\n", a);
    }
    virtual void undo() override {
        printf("----%d\n", a);
    }
};

class CommandSecond : public Command
{
public:
    CommandSecond() {
        printf("Construct CommandSecond\n");
    }
    virtual ~CommandSecond() override {
        printf("Destruct CommandSecond\n");
    }
    std::string b;
    virtual void execute() override {
        printf("%s\n", b.c_str());
    }
    virtual void undo() override {
        printf("----%s\n", b.c_str());
    }
};

class CommandManager
{
public:
    void push(std::unique_ptr<Command>&& command)
    {
        undone_commands = {};

        if (commands.size() >= MAX_COMMAND_COUNT)
        {
            commands.pop_back();
        }

        commands.push_front(std::move(command));
    }

    void undo()
    {
        if (commands.empty())
        {
            return;
        }

        auto command = std::move(commands.front());
        commands.pop_front();
        command->undo();
        undone_commands.push(std::move(command));
    }

    void redo()
    {
        if (undone_commands.empty())
        {
            return;
        }

        auto command = std::move(undone_commands.top());
        undone_commands.pop();
        command->execute();
        commands.push_front(std::move(command));
    }

private:
    std::deque<std::unique_ptr<Command>> commands;
    std::stack<std::unique_ptr<Command>> undone_commands;
};

int main()
{
    CommandManager command_manager;

    auto command_first = std::make_unique<CommandFirst>();
    command_first->a = 1;
    command_first->execute();
    command_manager.push(std::move(command_first));

    auto command_second = std::make_unique<CommandSecond>();
    command_second->b = "Hello";
    command_second->execute();
    command_manager.push(std::move(command_second));

    command_manager.undo();
    command_manager.undo();
    command_manager.undo();
    command_manager.redo();
    command_manager.redo();
    command_manager.redo();
    command_manager.undo();

    command_first = std::make_unique<CommandFirst>();
    command_first->a = 2;
    command_first->execute();
    command_manager.push(std::move(command_first));

    command_manager.undo();
    command_manager.undo();
    command_manager.undo();
    command_manager.redo();
    command_manager.redo();
    command_manager.redo();

    return EXIT_SUCCESS;
}


*/

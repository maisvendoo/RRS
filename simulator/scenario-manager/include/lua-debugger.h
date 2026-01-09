#ifndef     LUA_DEBUGGER_H
#define     LUA_DEBUGGER_H

#include    <sol/sol.hpp>
#include    <fstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Logger
{
    std::string filename;

    std::ofstream log_file;

public:

    Logger(const std::string &filename = "../logs/lua-debug.log")
        : filename(filename)
    {
        log_file.open(filename, std::ios::app);
    }

    ~Logger()
    {
        if (log_file.is_open())
        {
            log_file.close();
        }
    }

    void log_msg(const std::string &msg)
    {
        log_file << msg << std::endl;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LuaDebugger
{
public:

    LuaDebugger();

    ~LuaDebugger();

    void init(sol::state &lua);

private:

    static void debug_hook(lua_State *L, lua_Debug *ar);

    void install_hook(sol::state &lua, int mask = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE);
};

#endif

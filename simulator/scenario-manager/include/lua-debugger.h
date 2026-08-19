#ifndef     LUA_DEBUGGER_H
#define     LUA_DEBUGGER_H

#include    <sol/sol.hpp>

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

    static const uint64_t LOG_INSTANCE = 1;

    /// Инициализация лога трассировки
    void init_trace_journal();

    static void debug_hook(lua_State *L, lua_Debug *ar);

    void install_hook(sol::state &lua, int mask = LUA_MASKCALL |
                                                  LUA_MASKRET |
                                                  LUA_MASKLINE);
};

#endif

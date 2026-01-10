#include    <lua-debugger.h>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LuaDebugger::LuaDebugger()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LuaDebugger::~LuaDebugger()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LuaDebugger::init(sol::state &lua)
{
    install_hook(lua);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LuaDebugger::debug_hook(lua_State *L, lua_Debug *ar)
{
    static int call_depth = 0;
    const char *event_name = "";

    switch (ar->event)
    {
    case LUA_HOOKCALL:

        event_name = "CALL";

        break;

    case LUA_HOOKRET:

        event_name = "RETURN";

        break;

    case LUA_HOOKLINE:

        event_name = "LINE";

        break;

    case LUA_HOOKTAILCALL:

        event_name = "TAILCALL";

        break;
    }

    lua_getinfo(L, "nS1", ar);

    QString msg = QString("[LUA]: %1 %2 %3:%4 (func: %5)")
                      .arg(call_depth * 2)
                      .arg(event_name)
                      .arg(ar->source)
                      .arg(ar->currentline)
                      .arg(ar->name ? ar->name : "anonymous");

    Logger logger;

    logger.log_msg(msg.toStdString());

    if (ar->event == LUA_HOOKCALL)
        call_depth++;

    if (ar->event == LUA_HOOKRET || ar->event == LUA_HOOKTAILCALL)
        call_depth--;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LuaDebugger::install_hook(sol::state &lua, int mask)
{
    lua_sethook(lua.lua_state(), &debug_hook, mask, 0);
}



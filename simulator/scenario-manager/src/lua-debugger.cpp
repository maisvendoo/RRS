#include    <lua-debugger.h>
#include    <filesystem.h>
#include    <Journal.h>
#include    <JournalFile.h>
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
    init_trace_journal();
    install_hook(lua);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LuaDebugger::init_trace_journal()
{
    FileSystem &fs = FileSystem::getInstance();
    QString path = QString(fs.combinePath(fs.getLogsDir(), "lua-trace.log").c_str());

    Journal::instance(LOG_INSTANCE)->addStorage( new JournalFile(path, JournalLevel::All) );

    QString line = "";

    for (int i = 0; i < 80; ++i)
        line += "=";

    Journal::instance(LOG_INSTANCE)->message(" ");
    Journal::instance(LOG_INSTANCE)->message(line);
    Journal::instance(LOG_INSTANCE)->message("Started new Lua debug session");
    Journal::instance(LOG_INSTANCE)->message("Journal subsystem is initialized successfully");
    Journal::instance(LOG_INSTANCE)->message(line);
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

    Journal::instance(LOG_INSTANCE)->info(msg);

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



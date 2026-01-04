#include    <scenario-manager.h>
#include    <sol/sol.hpp>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScenarioManager::ScenarioManager(QObject *parent) : QObject(parent)
{
    sol::state lua;
    lua.open_libraries(sol::lib::base);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScenarioManager::~ScenarioManager()
{

}

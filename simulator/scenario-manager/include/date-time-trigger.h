#ifndef     DATE_TIME_TRIGGER_H
#define     DATE_TIME_TRIGGER_H

#include    <sol/sol.hpp>
#include    <datetime.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct date_time_trigger_t
{
    /// Момент времени активации триггера (с учетом даты!)
    simulator_time_t action_time;
    /// Функция Lua, исполняемая триггером
    sol::protected_function action_func;

    date_time_trigger_t()
    {

    }
};

#endif

#ifndef     CONTROL_PRIORITY_H
#define     CONTROL_PRIORITY_H

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum ControlPriority : std::int8_t
{
    /// Уровень "бога" - перекрывает всех
    CTRL_PRIORITY_GOT = 0,
    /// Приоритет аппаратного пульта
    CTRL_PRIORITY_HARDWARE = 1,
    /// Приоритет полноправного клиента ПК
    CTRL_PRIORITY_FULL = 2,
    /// Приоритет ограниченного клиента ПК (когда нибудь используем)
    CTRL_PRIORITY_LIMITED = 3,
    /// Гостевой приоритет - смотреть можно, но не управлять
    CTRL_PRIORITY_GUEST = 4
};

#endif

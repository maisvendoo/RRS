#ifndef     PASSCAR_IO_CONTROLLER_H
#define     PASSCAR_IO_CONTROLLER_H

#include    <io-controller.h>

//------------------------------------------------------------------------------
// Модуль ввода пассажирского вагона.
// У вагона нет кабин управления, поэтому все органы общие (CabinesNum = 0)
//------------------------------------------------------------------------------
class PassCarIOController : public IOController
{
public:

    PassCarIOController();

    ~PassCarIOController() = default;
};

#endif
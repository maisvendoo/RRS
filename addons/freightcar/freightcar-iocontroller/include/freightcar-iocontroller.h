#ifndef     FREIGHTCAR_IO_CONTROLLER_H
#define     FREIGHTCAR_IO_CONTROLLER_H

#include    <io-controller.h>

//------------------------------------------------------------------------------
// Модуль ввода грузового вагона.
// У вагона нет кабин управления, поэтому все органы общие (CabinesNum = 0)
//------------------------------------------------------------------------------
class FreightCarIOController : public IOController
{
public:

    FreightCarIOController();

    ~FreightCarIOController() = default;
};

#endif
//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------

#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::VL60() : Vehicle ()
  , Uks(30000.0f)
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , test_lamp(0.0)
  , sig(1)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::~VL60()
{

}

void VL60::step(double t, double dt)
{
    test_lamp += sig * 1.0f * dt;

    if (test_lamp > 1.0f)
    {
        sig = -1;

    }

    if (test_lamp < 0.0f)
    {
        sig = 1;
    }



    analogSignal[33] = 0.0;
    analogSignal[31] = 0.0;
    analogSignal[32] = 0.0;

    analogSignal[37] = Uks / 30000.0f;

    analogSignal[40] = pant1_pos;
    analogSignal[41] = pant2_pos;
    analogSignal[42] = gv_pos;

    analogSignal[47] = cut(test_lamp, 0.0f, 1.0f);

    analogSignal[60] = 1.0f;
}

void VL60::keyProcess()
{
    if (getKeyState(KEY_O))
    {
        if (isShift())
            pant1_pos = 0.8f;
        else
            pant1_pos = 0.0f;
    }

    if (getKeyState(KEY_I))
    {
        if (isShift())
            pant2_pos = 0.8f;
        else
            pant2_pos = 0.0f;
    }

    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_pos = 1.0f;
        else
            gv_pos = 0.0f;
    }
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)

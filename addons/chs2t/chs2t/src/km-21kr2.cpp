#include "km-21kr2.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Km21KR2::Km21KR2(QObject* parent) : Device(parent)
  , k21(true)
  , k22(true)
  , k23(false)

  , k01(false)
  , k02(false)

  , n(0)
  , p(0)
  , re(0)
  , hod(0)
  , fieldStep(0)
  , reverseState(0)
  , mainShaftPos(0.0)

{
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
Km21KR2::~Km21KR2()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void Km21KR2::load_config(CfgReader& cfg)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::preStep(state_vector_t& Y, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_W) && reverseState != 1 && re == 0)
    {
        reverseState += 1;
        re = 1;
    }
    if (getKeyState(KEY_S) && reverseState != -1 && re == 0)
    {
        reverseState -= 1;
        re = 1;
    }
    if (!getKeyState(KEY_W) && !getKeyState(KEY_S))
        re = 0;

    if (reverseState == 0)
        return;

    // Авт. сброс
    if (getKeyState(KEY_D))
    {
        if (isControl())
        {
            n = 0;
        }
        else
        {
            if (isShift() && fieldStep != 0 && p == 0 && hod)
            {
                fieldStep -= 1;
                p = 1;
            }
            else if (n == 0 && !isShift())
            {
                k21 = false;
                k22 = false;
                k23 = false;

                mainShaftPos = 0.5;
            }
        }
    }

    // 1 вниз
    if (getKeyState(KEY_A))
    {
        if(isShift() && fieldStep != 5 && p == 0 && hod)
        {
            fieldStep += 1;
            p = 1;
        }
        else if(n == 0 && !isShift())
        {
            k21 = false;
            k22 = true;
            k23 = true;

            mainShaftPos = -0.5;
        }
    }

    if (n == 1)
    {
        return;
    }

    // Авт. набор
    if (getKeyState(KEY_Q))
    {
        k21 = true;
        k22 = true;
        k23 = true;

        mainShaftPos = -1.0;
    }

    // 1 вверх
    if (getKeyState(KEY_E))
    {
        k21 = true;
        k22 = false;
        k23 = false;
        n = 1;

        mainShaftPos = 1.0;
    }

    // Ноль
    if (!getKeyState(KEY_Q) && !getKeyState(KEY_E) &&
        !getKeyState(KEY_A) && !getKeyState(KEY_D))
    {
        p = 0;
        k21 = true;
        k22 = true;
        k23 = false;

        mainShaftPos = 0.0;
    }

    controlState.up = k21 * k23;
    controlState.up1 = !k21 * k23;
    controlState.zero = k22 * !k23;
    controlState.down1 = !k21 * !k22;
    controlState.down = k21 * !k22;
}

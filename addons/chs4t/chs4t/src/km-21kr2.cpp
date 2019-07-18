//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 16/06/2019
//
//------------------------------------------------------------------------------

#include "km-21kr2.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Km21KR2::Km21KR2(QObject* parent) : Device(parent)
  , up(false)
  , up1(false)
  , zero(true)
  , down1(false)
  , down(false)
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
//    int zu = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::stepKeysControl(double t, double dt)
{
    up = getKeyState(KEY_Q);
    up1 = getKeyState(KEY_A);

    if (getKeyState(KEY_E))
    {
        down = true;
    }
    if (getKeyState(KEY_D) && isControl())
    {
        down = false;
    }
    if (getKeyState(KEY_D) && !isControl())
    {
        down1 = true;
    }
    else
    {
        down1 = false;
    }

    if (!up && !up1 && !down1 && !down)
    {
        zero = true;
    }
    else
    {
        zero = false;
    }

    double s01 = static_cast<double>(up);
    double s02 = static_cast<double>(up1);
    double s03 = static_cast<double>(zero);
    double s04 = static_cast<double>(down1);
    double s05 = static_cast<double>(down);

    controlState.a2b2 = s01 + s03 + s05;
    controlState.c2d2 = s01 + s02 + s03;
    controlState.e2f2 = s01 + s02;
    controlState.i2g2 = s03 + s04 + s05;
    controlState.j2k2 = s04 + s05;
}

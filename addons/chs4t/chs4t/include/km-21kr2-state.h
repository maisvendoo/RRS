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
#ifndef KM21KR2STATE_H
#define KM21KR2STATE_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct ControllerState
{
    double a2b2;
    double c2d2;
    double e2f2;
    double i2g2;
    double j2k2;

    ControllerState()
        : a2b2(0)
        , c2d2(0)
        , e2f2(0)
        , i2g2(0)
        , j2k2(0)
    {

    }
};

#endif // KM21KR2STATE_H

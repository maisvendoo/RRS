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


#include "stepswitch.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
StepSwitch::StepSwitch(QObject* parent) : Device(parent)
//  , s62(0)
//  , s67(0)
//  , s69(0)
//  , s45(0)
//  , s43(0)
//  , s410(0)
//  , shaft_rel(0)
//  , PI(Physics::PI)
//  , p1(0)
//  , p2(PI / 2.0)
//  , p3(PI)
//  , p4(3.0 * PI / 2.0)
//  , KL(0)
//  , UV(0)
//  , HG(0)
//  , v1(0)
//  , v2(0)
//  , u(0)
  , V(2.96)
  , poz_d(0)
  , poz(0)
  , n(0)

{
    /*rs = new Trigger()*/;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
StepSwitch::~StepSwitch()
{

}

//------------------------------------------------------------------------------
//A2B2
//------------------------------------------------------------------------------
void StepSwitch::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
//    if (ctrlState.a2b2 || ctrlState.e2f2)
//        rs->set();

//    if (1.0 - KL)
//        rs->reset();

//    double s01 = static_cast<double>(rs->getState());

//    double s1 = s01 * KL;

//    double s2 = s1 * ctrlState.c2d2 * UV;

//    s62 = 1.0 - s2;
//    s67 = s2;
//    s69 = s2;

//    //-----------------------------------------------

//    if (ctrlState.a2b2 || ctrlState.j2k2)
//        rs->set();

//    if (1.0 - NM)
//        rs->reset();

//    s01 = static_cast<double>(rs->getState());

//    s1 = s01 * NM;

//    s2 = s1 * ctrlState.i2g2 * HG;

//    s45 = 1.0 - s2;
//    s43 = s2;
//    s410 = s2;

//    //----------------------------------------

//    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    KL = ((shaft_rel >= p1 && shaft_rel <= p2) ||
//          (shaft_rel >= p3 && shaft_rel <= p4));
//    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//    AB = (shaft_rel >= p1 && shaft_rel <= p2);

//    GH = (shaft_rel == p3);

//    CD = (shaft_rel >= p2 && shaft_rel <= p3);

//    EF = (shaft_rel == p1 || shaft_rel == p4);

//    IJ = (shaft_rel >= p3 && shaft_rel <= p4);

//    NM = ((shaft_rel == p1) ||
//          (shaft_rel >= p2 && shaft_rel <= p3) ||
//          (shaft_rel == p4));

//    double s3 = s62 * GH;

//    v1 = s67 * AB + s3;
//    if (v1 > 1)
//        v1 = 1;

//    v2 = s69 * CD + s3;
//    if (v2 > 1)
//        v2 = 1;

//    if (ctrlState.e2f2)
//        u = (v1 != v2);
//    if (ctrlState.j2k2)
//        u = -(v1 != v2);

//    dYdt[0] = ang_vel * u * hs_p(32 * PI - Y[0]) * hs_p(Y[0]);

//    shaft_rel = Y[0] - static_cast<int>(Y[0] / (2 * PI)) * 2 * PI;

//    poz = static_cast<int>(Y[0] / PI);

//    UV = hs_p(32 - poz);

//    HG = hs_p(poz);








}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void StepSwitch::load_config(CfgReader& cfg)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::preStep(state_vector_t& Y, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StepSwitch::stepKeysControl(double t, double dt)
{
    if (ctrlState.up)
    {
        poz_d += V * hs_p(32 - poz_d) * dt;
        n = 0;
    }
    if (ctrlState.up1 && poz_d < 32 && n == 0)
    {
        poz += 1;
        poz_d = poz;
        n = 1;
    }
    if (ctrlState.down1 && poz_d > 0 && n == 0)
    {
        poz -= 1;
        poz_d = poz;
        n = 1;
    }
    if (ctrlState.down)
    {
        poz_d -= V * hs_p(poz_d) * dt;
        n = 0;
    }
    if (ctrlState.zero)
    {
        n = 0;
    }

    poz = static_cast<int>(poz_d);

}

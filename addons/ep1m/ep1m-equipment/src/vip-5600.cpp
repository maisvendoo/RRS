#include    "vip-5600.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RectInvertConverter::RectInvertConverter(QObject *parent) : Device(parent)
  , r(0.03)
  , U1(0.0)
  , U2(0.0)
  , U3(0.0)
  , K_rect(0.9)
  , alpha(90.0)
  , zoneNum(ZONE1)
  , U_out(0.0)
  , I_out(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RectInvertConverter::~RectInvertConverter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RectInvertConverter::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // 1-я зона
    zone[ZONE1].Umin = 0;
    zone[ZONE1].Umax = U1;

    // 2-я зона
    zone[ZONE2].Umin = U1;
    zone[ZONE2].Umax = U1 + U2;

    // 3-я зона
    zone[ZONE3].Umin = U3;
    zone[ZONE3].Umax = U3 + U2;

    // 4-я зона
    zone[ZONE4].Umin = U3 + U2;
    zone[ZONE4].Umax = U3 + U2 + U1;

    // Вычисляем ЭДС по номеру зоны и углу
    double Umin = zone[zoneNum - 1].Umin;
    double Umax = zone[zoneNum - 1].Umax;

    double E_out = (Umin  + (Umax - Umin) * cos(alpha * Physics::PI / 180.0)) * K_rect;

    // Вычисляем выходное напряжение с учетом тока нагрузки
    U_out = E_out - r * I_out;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RectInvertConverter::ode_system(const state_vector_t &Y,
                                     state_vector_t &dYdt,
                                     double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RectInvertConverter::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "InternalRes", r);
    cfg.getDouble(secName, "K_rect", K_rect);
}

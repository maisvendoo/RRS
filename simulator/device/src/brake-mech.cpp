//------------------------------------------------------------------------------
//
//      Vehicle brake mechanism abstract class
//      (c) maisvendoo, 15/02/2019
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Vehicle brake mechanism abstract class
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 15/02/2019
 */

#include    "brake-mech.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeMech::BrakeMech(QObject *parent) : Device(parent)  
  , shoesAxis(4)
  , cylNum(1)
  , Q(0.0)
  , brakeTorque(0.0)
  , V0(0.0)
  , S(0.1)
  , cylDiam(0.356)
  , Lmax(0.085)
  , K(0.0)
  , effRadius(0.475)
  , wheelDiameter(0.95)
  , velocity(0.0)
  , shoeType("iron")
  , Kmax(3.0)
  , p_max(0.4)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeMech::~BrakeMech()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setAirFlow(double Q)
{
    this->Q = Q;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setEffFricRadius(double radius)
{
    effRadius = radius;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setWheelDiameter(double diameter)
{
    wheelDiameter = diameter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setVelocity(double v)
{
    velocity = v;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getBrakeTorque() const
{
    return brakeTorque;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getBrakeCylinderPressure() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getShoeForce() const
{
    return K;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    double fullVolume = (V0 + S * Lmax) * cylNum;
    dYdt[0] = Q / fullVolume;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    // Вычисляем нажатие на одну колодку (кгс)
    K = Kmax * Y[0] / p_max;

    // Вычисляем тормозную силу, получаемую от одной колодки
    double shoe_brake_force = K * phi(K, velocity) * Physics::g * 1000.0;

    // Вычисляем момент на оси колесной пары
    brakeTorque = shoesAxis * shoe_brake_force * effRadius;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "ShoeType", shoeType);
    cfg.getInt(secName, "ShoesAxis", shoesAxis);
    cfg.getInt(secName, "CylNum", cylNum);
    cfg.getDouble(secName, "DeadVolume", V0);

    cfg.getDouble(secName, "CylinderDiameter", cylDiam);
    S = Physics::PI * cylDiam * cylDiam / 4.0;

    cfg.getDouble(secName, "StockOut", Lmax);

    cfg.getDouble(secName, "Kmax", Kmax);
    cfg.getDouble(secName, "p_max", p_max);

    double p_cyl_begin = 0.0;
    cfg.getDouble(secName, "InitPressure", p_cyl_begin);

    setY(0, p_cyl_begin);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::phi(double K, double v)
{
    double fric_coeff = 0.0;

    double V = Physics::kmh * abs(v);

    if (shoeType == "iron")
    {
        fric_coeff = 0.6 * (16 * K + 100) *(V + 100) / (80 * K + 100) / (5 * V + 100);
    }

    if (shoeType == "iron-ph")
    {
        fric_coeff = 0.5 * (16 * K + 100) * (V + 100) / (52 * K + 100) / (5 * V + 100);
    }

    if (shoeType == "composite")
    {
        double k = 2 * effRadius / wheelDiameter;
        fric_coeff = 0.44 * (K + 20) * (V * k + 150) / (4 * K + 20) / (2 * V * k + 150);
    }

    return fric_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeMech *loadBrakeMech(QString lib_path)
{
    BrakeMech *brake_mech = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetBrakeMech getBrakeMech = reinterpret_cast<GetBrakeMech>(lib.resolve("getBrakeMech"));

        if (getBrakeMech)
        {
            brake_mech = getBrakeMech();
        }
    }

    return brake_mech;
}

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
BrakeMech::BrakeMech(size_t num, QObject *parent) : Device(parent)
  , cyl_num(1)
  , cyl_d(0.356)
  , S(Physics::PI * cyl_d * cyl_d / 4.0)
  , stock_out_max(0.085)
  , stock_out_coeff(0.0)
  , stock_out_cur(0.0)
  , V0(1.0e-3)
  , V((V0 + S * stock_out_max) * cyl_num)
  , Q(0.0)
  , k_leak(0.0)
  , p_begin(0.02)
  , p_end(0.04)
  , p_max(0.4)
  , Kmax(3.0)
  , Kcoef(0.0)
  , K(0.0)
  , shoes_per_axis(2)
  , wheel_r(0.475)
  , eff_r(0.475)
  , shoeType("iron")
{
    setAxisNum(num);
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
void BrakeMech::setBCflow(double value)
{
    Q = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setLeakCoeff(double value)
{
    k_leak = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getBCpressure() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getStockOutCoeff() const
{
    return stock_out_coeff;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getStockOut() const
{
    return stock_out_cur;
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
void BrakeMech::setAxisNum(size_t num)
{
    axis_num = num;
    axis_omega.resize(axis_num);
    brake_torque.resize(axis_num);
    std::fill(axis_omega.begin(), axis_omega.end(), 0.0);
    std::fill(brake_torque.begin(), brake_torque.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setWheelRadius(double value)
{
    wheel_r = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setEffFricRadius(double value)
{
    eff_r = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::setAngularVelocity(size_t idx, double value)
{
    if (idx < axis_omega.size())
        axis_omega[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeMech::getBrakeTorque(size_t idx) const
{
    if (idx < brake_torque.size())
        return brake_torque[idx];
    return 0.0;
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

    dYdt[0] = (Q - k_leak * Y[0]) / V;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    // Рассчитываем положение штока
    stock_out_coeff = cut((Y[0] - p_begin) / (p_end - p_begin), 0.0, 1.0);
    stock_out_cur = stock_out_coeff * stock_out_max;

    // Если шток вышел полностью - считаем тормозное нажатие
    if ((Y[0] - p_end) > Physics::ZERO)
    {
        // Вычисляем нажатие на одну колодку (кгс)
        Kcoef = (Y[0] - p_end) / (p_max - p_end);
        K = Kcoef * Kmax;

        for (size_t i = 0; i < axis_num; ++i)
        {
            double v = axis_omega[i] * wheel_r;

            // Вычисляем тормозную силу, получаемую от одной колодки (Н)
            double shoe_brake_force = K * phi(K, v) * Physics::g * 1000.0;

            // Вычисляем момент на оси колесной пары (Н * м)
            brake_torque[i] = shoes_per_axis * shoe_brake_force * eff_r;
        }
    }
    else
    {
        Kcoef = 0.0;
        K = 0.0;
        std::fill(brake_torque.begin(), brake_torque.end(), 0.0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeMech::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    int num = 0;
    cfg.getInt(secName, "AxisNum", num);
    if (num > 0)
        setAxisNum(static_cast<size_t>(num));

    cfg.getInt(secName, "CylindersNum", cyl_num);
    cfg.getDouble(secName, "CylindersDiameter", cyl_d);
    S = Physics::PI * cyl_d * cyl_d / 4.0;

    cfg.getDouble(secName, "StockOut", stock_out_max);

    double tmp = 0.0;
    cfg.getDouble(secName, "DeadVolume", tmp);
    if (tmp > 1e-3)
        V0 = tmp;
    V = (V0 + S * stock_out_max) * cyl_num;

    cfg.getDouble(secName, "kLeak", k_leak);

    cfg.getDouble(secName, "pBegin", p_begin);
    cfg.getDouble(secName, "pEnd", p_end);
    cfg.getDouble(secName, "pMax", p_max);

    cfg.getDouble(secName, "Kmax", Kmax);
    cfg.getInt(secName, "ShoesAxis", shoes_per_axis);
    cfg.getDouble(secName, "EffRadius", eff_r);
    cfg.getString(secName, "ShoeType", shoeType);

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
        double k = eff_r / wheel_r;
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

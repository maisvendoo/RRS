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

#ifndef     BRAKE_MECH_H
#define     BRAKE_MECH_H

#include    "device.h"

/*!
 * \class
 * \brief Brake mechanics
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeMech : public Device
{
public:

    BrakeMech(size_t axis_num = 4, QObject *parent = Q_NULLPTR);

    ~BrakeMech();

    /// Задать поток воздуха в тормозные цилиндры
    void setBCflow(double value);

    /// Задать коэффициент утечек из тормозных цилиндров в атмосферу
    void setLeakCoeff(double value);

    /// Давление в тормозных цилиндрах
    double getBCpressure() const;

    /// Текущий относительный выход штока
    /// (от 0.0 - отпускное положение до 1.0 - тормозное положение)
    double getStockOutCoeff() const;

    /// Текущий выход штока, м
    double getStockOut() const;

    /// Текущая относительная сила нажатия на тормозную колодку
    /// (от 0.0 - нет нажатия до 1.0 - нажатие при максимальном давлении в ТЦ)
    double getShoeForceCoeff() const;

    /// Текущая сила нажатия на тормозную колодку, тс
    double getShoeForce() const;

    /// Задать число осей в тормозной рычажной передаче
    void setAxisNum(size_t num);

    /// Задать радиус поверхности катания колёс
    void setWheelRadius(double value);

    /// Задать эффективный радиус тормозной поверхности
    /// (радиус поверхности катания колёс или тормозных дисков)
    void setEffFricRadius(double value);

    /// Задать текущую угловую скорость колёсной пары № idx
    void setAngularVelocity(size_t idx, double value);

    /// Момент тормозных сил на оси № idx, Н * м
    double getBrakeTorque(size_t idx) const;

protected:

    /// Число осей в тормозной рычажной передаче
    size_t  axis_num;

    /// Число тормозных цилиндров в тормозной рычажной передаче
    int  cyl_num;

    /// Диаметр тормозного цилиндра, м
    double  cyl_d;

    /// Площадь поршня
    double  S;

    /// Максимальный выход штока, м
    double  stock_out_max;

    /// Текущий относительный выход штока
    double  stock_out_coeff;

    /// Текущий выход штока, м
    double  stock_out_cur;

    /// Мёртвый объём тормозных цилиндров и подводящих труб, м^3
    double  V0;

    /// Текущий объём тормозных цилиндров и подводящих труб
    double  V;

    /// Поток в тормозные цилиндры
    double  Q;

    /// Коэффициент утечек из тормозных цилиндров
    double  k_leak;

    /// Давление в ТЦ - начало движения поршня из отпускного положения
    double  p_begin;

    /// Давление в ТЦ - окончание движения поршня в тормозное положение
    double  p_end;

    /// Максимальное давление в ТЦ
    double  p_max;

    /// Максимальное нажатие на колодку (при максимальном давлении), тс
    double  Kmax;

    /// Текущая относительная сила нажатия на колодку
    double  Kcoef;

    /// Текущая сила нажатия на колодку, тс
    double  K;

    /// Число колодок на каждой оси
    int  shoes_per_axis;

    /// Текущие угловые скорости каждой оси
    std::vector<double>  axis_omega;

    /// Радиус колёс
    double wheel_r;

    /// Эффективный радиус тормозной поверхности
    double eff_r;

    /// Тормозной момент на осях
    std::vector<double>  brake_torque;

    /// Материал колодок: iron/iron-ph/composite (чугун/фосфористый чугун/композит)
    QString shoeType;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

protected:

    /// Load configuration
    virtual void load_config(CfgReader &cfg);

    /// Friction coefficient
    double phi(double K, double v);
};

/*!
 * \typedef
 * \brief Get brake mechics model function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeMech* (*GetBrakeMech)();

/*!
 * \def
 * \brief Macro for generation getBrakeMech() function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_MECH(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeMech *getBrakeMech() \
    { \
        return new (ClassName) (); \
    }

/*!
 * \fn
 * \brief Loading of brake mechanism module
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeMech *loadBrakeMech(QString lib_path);

#endif // BRAKEMECH_H

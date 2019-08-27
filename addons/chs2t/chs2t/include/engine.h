//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef ENGINE_H
#define ENGINE_H

#include "device.h"
#include "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Engine : public Device
{
public:

    ///Конструктор
    Engine(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Engine();

    void setPoz(int poz) { this->poz = poz; }

    void setR(double R)  { this->R_r = R; }

    void setOmega(double value) { omega = value; }

    void setU(double value) { U = value; }

    double getTorque() const { return  torque; }

    double getIa() const { return getY(0); }

    double getIf() const { return getIa() * beta; }

    double getUd() const { return U - getIa() * R_r; }

    void setBeta(double value) { beta = value; }

    void setBetaStep(int step);

    void setDirection(int revers_state);

    double getBeta() { return beta; }

private:

    int    poz;

    int    n;

    /// Степень ослабления возбуждения
    double  beta;

    /// Сопротивление обмотки якоря (ОЯ)
    double  R_a;

    /// Сопротивление главных полюсов ОВ
    double  R_gp;

    /// Сопротивление добавочных полюсов
    double  R_dp;

    ///  Сопротивление реостата
    double  R_r;

    /// Эквивадентная индуктивность обмоток
    double  L_af;

    /// Угловая скорость вала
    double  omega;

    /// Момент на валу
    double  torque;

    /// Напряжение
    double  U;

    double  omega_nom;

    int     direction;

    MotorMagneticChar cPhi;

    QMap<int, double> fieldStep;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
    void load_config(CfgReader &cfg);
    void preStep(state_vector_t &Y, double t);
    void stepKeysControl(double t, double dt);

    double calcCPhi(double I);



};

#endif // ENGINE_H

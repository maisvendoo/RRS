#ifndef     ENGINE_H
#define     ENGINE_H

#include    "device.h"
#include    "motor-magnetic-char.h"
#include    "ampermeters-state.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Motor : public Device
{
public:

    ///Конструктор
    Motor(QObject *parent = Q_NULLPTR);

    ///Деструктор
    ~Motor();

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

    void setAmpermetersState(ampermeters_state_t state) { amp_state = state; }

    double getBeta() { return beta; }

    double getI12() const;

    double getI34() const;

    double getI56() const;

private:

    /// Номер позиции
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

    /// Направление
    int     direction;

    MotorMagneticChar cPhi;

    QMap<int, double> fieldStep;

    ampermeters_state_t amp_state;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка данных из конфигурационных файлов
    void load_config(CfgReader &cfg);

    void preStep(state_vector_t &Y, double t);    

    double calcCPhi(double I);
};

#endif // ENGINE_H

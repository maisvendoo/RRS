#ifndef     STARTER_GENERATOR_H
#define     STARTER_GENERATOR_H

#include    "device.h"

#include    "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StarterGenerator : public Device
{
public:

    StarterGenerator(QObject *parent = Q_NULLPTR);

    ~StarterGenerator();

    void init(QString magnetic_char_file);

    void step(double t, double dt);

    void setMotorMode(bool is_motor);

    void setLoadCurrent(double In);

    void setOmega(double omega);

    void setAncorVoltage(double Ua) { this->Ua = Ua; }

    void setFieldVoltage(double Uf) { this->Uf = Uf; }

    double getVoltage() const;

    double getTorque();

    double getAncorCurrent() const { return Ia; }

private:

    /// Напряжение на якоре
    double  Ua;

    /// Напряжение на обмотке возбуждения (в режиме генератора)
    double  Uf;

    /// Ток якоря
    double  Ia;

    /// Ток возбуждения
    double  If;

    /// Сопротивление обмотки якоря
    double  Ra;

    /// Сопротивление обмотки возбуждения
    double  Rf;

    /// Постоянная времени нарастания тока якоря
    double  Ta;

    /// Постоянная времени нарастания тока возбуждения
    double  Tf;

    /// Угловая скорость вращения вала
    double  omega;

    /// Момент на валу
    double  M;

    /// Признак двигательного режима
    bool    is_motor;

    /// Ток нагрузки в генераторном режиме
    double  In;

    /// Напряжение, вырабатываемое в генераторном режиме
    double  U;

    /// Флаг запуска
    bool is_started;

    /// Тамер переключения схемы
    Timer   *switch_timer;

    /// Характеристика намагничивания обмотки возбуждения
    MotorMagneticChar   magnetic_char;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void preStep_motor(state_vector_t &Y, double t);

    void preStep_generator(state_vector_t &Y, double t);

    void ode_system_motor(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void ode_system_generator(const state_vector_t &Y, state_vector_t &dYdt, double t);

    double cPhi(double If);

private slots:

    void slotSwitchMode();
};

#endif // STARTER_GENERATOR_H

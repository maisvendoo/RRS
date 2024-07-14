#ifndef     DC_MOTOR_FAN_H
#define     DC_MOTOR_FAN_H

#include    "device.h"

//------------------------------------------------------------------------------
// Мотор-вентилятор постоянного тока
//------------------------------------------------------------------------------
class DEVICE_EXPORT DCMotorFan : public Device
{
public:

    DCMotorFan(QObject *parent = Q_NULLPTR);

    ~DCMotorFan();

    /// Задать напряжение
    void setPowerVoltage(double value);

    /// Потребляемый ток
    double getPowerCurrent() const;

    /// Текущее состояние
    bool isPowered() const;

    /// Текущее обеспечение потока воздуха
    bool isReady() const;

    /// Состояние звука вентилятора
    sound_state_t getSoundState() const;
    void RegulateSoundByOnOff(bool value);
    void RegulateSoundByPitch(bool value);

protected:

    /// Питающее напряжение
    double  U_power;

    /// Номинальное напряжение
    double  U_nom;

    /// Ток
    double  I;

    /// Номинальная угловая скорость вращения
    double  omega0;

    /// Сопротивление двигателя
    double  R;

    /// Харакстеристика магнитного поля двигателя
    double  cPhi;

    /// Момент инерции ротора
    double  J;

    /// Коэффициент аэродинамического сопротивления вращению ротора
    double  kf;

    /// Признак включения
    bool    is_powered;

    /// Признак обеспечения потока воздуха
    bool    is_ready;

    /// Состояние звука вентилятора
    sound_state_t sound_state;
    bool    reg_sound_by_on_off;
    bool    reg_sound_by_pitch;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // DC_MOTOR_FAN_H

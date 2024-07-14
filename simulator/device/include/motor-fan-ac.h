#ifndef     AC_MOTOR_FAN_H
#define     AC_MOTOR_FAN_H

#include    "device.h"

//------------------------------------------------------------------------------
// Мотор-вентилятор переменного тока
//------------------------------------------------------------------------------
class DEVICE_EXPORT ACMotorFan : public Device
{
public:

    ACMotorFan(QObject *parent = Q_NULLPTR);

    ~ACMotorFan();

    /// Задать питающее напряжение
    void setPowerVoltage(double value);

    /// Текущее состояние питания
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

    /// Номинальная угловая скорость магнитного поля статора
    double  omega0;

    /// Максимальный момент
    double  Mmax;

    /// Критическое скольжение
    double  s_kr;

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

#endif // AC_MOTOR_FAN_H

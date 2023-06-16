#ifndef     AC_MOTOR_COMPRESSOR_H
#define     AC_MOTOR_COMPRESSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT ACMotorCompressor : public Device
{
public:

    ACMotorCompressor(QObject *parent = Q_NULLPTR);

    ~ACMotorCompressor();

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать питающее напряжение
    void setPowerVoltage(double value);

    /// Текущее состояние
    bool isPowered() const;

    void setSoundName(const QString &value);
    void RegulateSoundByOnOff(bool value);
    void RegulateSoundByPitch(bool value);

protected:

    /// Внешнее противодавление
    double  pFL;

    /// Расход возхуа
    double  QFL;

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

    /// Момент сопротивления вращению ротора
    double  Mxx;

    /// Коэффициент перехода от скорости ротора к производимому давлению
    double  K_pressure;

    /// Коэффициент потока в питательную магистраль
    double  K_flow;

    /// Признак включения
    bool    is_powered;

    QString soundName;
    bool    reg_sound_by_on_off;
    bool    reg_sound_by_pitch;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // MOTOR_COMPRESSOR_H

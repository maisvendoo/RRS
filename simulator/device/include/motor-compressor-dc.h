#ifndef     DC_MOTOR_COMPRESSOR_H
#define     DC_MOTOR_COMPRESSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT DCMotorCompressor : public Device
{
public:

    DCMotorCompressor(QObject *parent = Q_NULLPTR);

    ~DCMotorCompressor();

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать напряжение
    void setPowerVoltage(double value);

    /// Потребляемый ток
    double getPowerCurrent() const;

    /// Текущее состояние
    bool isPowered() const;

    void setSoundName(const QString &value);
    void RegulateSoundByOnOff(bool value);
    void RegulateSoundByPitch(bool value);

private:

    /// Внешнее противодавление
    double  pFL;

    /// Расход возхуа
    double  QFL;

    /// Питающее напряжение
    double  U_power;

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

#endif // DC_MOTOR_COMPRESSOR_H

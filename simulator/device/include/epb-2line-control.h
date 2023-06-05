//------------------------------------------------------------------------------
//
//      Блок управления двухпроводным электропневматическим тормозом
//
//------------------------------------------------------------------------------
#ifndef     EPB_2LINE_CONTROL_H
#define     EPB_2LINE_CONTROL_H

#include    "device.h"

class DEVICE_EXPORT EPBControl : public Device
{
public:

    EPBControl(QObject *parent = Q_NULLPTR);

    ~EPBControl();

    /// Задать входное напряжение от преобразователя питания
    void setInputVoltage(double value);

    /// Задать частоту переменного тока от преобразователя питания
    void setInputFrequency(double value);

    /// Задать состояние перекрыши
    void setHoldState(bool is_hold);

    /// Задать состояние торможения
    void setBrakeState(bool is_brake);

    /// Выходное напряжение в рабочую линию ЭПТ
    double getWorkVoltage() const;

    /// Частота переменного тока в рабочую линию ЭПТ
    double getWorkFrequency() const;

    /// Задать напряжение в контрольной линии ЭПТ
    void setControlVoltage(double value);

    /// Задать частоту в контрольной линии ЭПТ
    void setControlFrequency(double value);

    /// Состояние контрольной лампы ЭПТ в режиме отпуска
    bool stateReleaseLamp() const;

    /// Состояние контрольной лампы ЭПТ в режиме перекрыши
    bool stateHoldLamp() const;

    /// Состояние контрольной лампы ЭПТ в режиме торможения
    bool stateBrakeLamp() const;

    virtual void step(double t, double dt);

private:

    /// Напряжение питания линии управления
    double U;

    /// Частота переменного тока для контроля линий управления ЭПТ
    double f;

    /// Состояние перекрыши
    bool is_hold;

    /// Состояние торможения
    bool is_brake;

    /// Величина напряжения в линии управления ЭПТ
    double work_U;

    /// Частота переменного напряжения в линии управления ЭПТ
    double work_f;

    /// Величина напряжения в контрольной линии ЭПТ
    double control_U;

    /// Частота переменного напряжения в контрольной линии ЭПТ
    double control_f;

    /// Состояние лампы "О" (отпуск)
    bool lampRelease;

    /// Состояние лампы "П" (перекрыша)
    bool lampHold;

    /// Состояние лампы "Т" (торможение)
    bool lampBrake;

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // EPB_2LINE_CONTROL_H

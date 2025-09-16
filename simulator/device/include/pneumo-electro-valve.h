#ifndef     PNEUMO_ELECTRO_VALVE_H
#define     PNEUMO_ELECTRO_VALVE_H

#include    "device.h"

//------------------------------------------------------------------------------
// Электромагнитный пневматический вентиль
//------------------------------------------------------------------------------
class /*DEVICE_EXPORT*/ PneumoElectroValve : public Device
{
public:

    /// Конструктор
    PneumoElectroValve(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~PneumoElectroValve();

    /// Задать напряжение, подаваемое на обмотку реле, В
    void setVoltage(double value);

    /// Ток, потребляемый реле, А
    double getCurrent() const;

    /// Cостояние электромагнитного пневматического вентиля
    bool isOpened() const;

    /// Задать давление со стороны магистрали
    void setPipePressure(double value);

    /// Давление со стороны оборудования
    double getPressureToDevice() const;

    /// Задать поток из оборудования
    void setDeviceFlow(double value);

    /// Поток в магистраль
    double getFlowToPipe() const;

    enum {
        NUM_SOUNDS = 4,
        CHANGE_SOUND = 0,   ///< Звук переключения электромагнитного пневмовентиля
        OPEN_SOUND = 1,     ///< Звук открытия электромагнитного пневмовентиля
        CLOSE_SOUND = 2,    ///< Звук перекрытия электромагнитного пневмовентиля
        DRAIN_FLOW_SOUND = 3///< Звук выхода воздуха через атмосферное отверстие
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = CHANGE_SOUND) const;

private:

    enum {
        PRESSURE = 0,   ///< Y[0] - Давление для оборудования за пневмовентилем
        CURRENT = 1     ///< Y[1] - Ток в катушке электромагнитного пневмовентиля
    };

    /// Давление в магистрали
    double p = 0.0;

    /// Поток из оборудования в магистраль
    double Q = 0.0;

    /// Поток выхода воздуха из оборудования через атмосферное отверстие
    double Q_atm = 0.0;

    /// Условный объём труб между краном и оборудованием
    double  V0 = 1.0e-3;

    /// Коэффициент к потоку выхода воздуха из оборудования в атмосферу при закрытом кране
    double K_atm = 0.0;

    /// Коэффициент громкости озвучки к потоку разрядки
    double K_sound = 2.0;

    /// Напряжение, подаваемое на обмотку
    double  U = 0.0;

    /// Номинальное рабочее напряжение
    double  U_nom = 50.0;

    /// Активное сопротивление якоря реле
    double  R = 170.0;

    /// Постоянная времени обмотки
    double  T = 0.1;

    /// Максимальный уровень отключения (0.0 - 1.0)
    double  level_off = 0.6;

    /// Минимальный уровень включения (0.0 - 1.0)
    double  level_on = 0.7;

    /// Гистерезис минимального тока включения - максимального тока отключения
    Hysteresis hysteresis = Hysteresis(level_off * U_nom / R, level_on * U_nom / R, false);

    /// Звук выхода воздуха из оборудования через атмосферное отверстие
    sound_state_t atm_flow_sound = sound_state_t();

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_ELECTRO_VALVE_H

#ifndef     PNEUMO_SHUTOFF_VALVE_H
#define     PNEUMO_SHUTOFF_VALVE_H

#include    "device.h"

//------------------------------------------------------------------------------
// Разобщительный кран
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoShutoffValve : public Device
{
public:

    /// Конструктор
    PneumoShutoffValve(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~PneumoShutoffValve();

    /// Задать управляющую клавишу для открытия крана
    void setKeySymbolOpen(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для открытия крана
    void setKeyModifierOpen(std::uint16_t key_modifier);

    /// Задать управляющую клавишу для закрытия крана
    void setKeySymbolClose(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для закрытия крана
    void setKeyModifierClose(std::uint16_t key_modifier);

    void setControl(std::set<uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    /// Закрыть разобщительный кран
    void close();

    /// Открыть разобщительный кран
    void open();

    /// Cостояние разобщительного крана
    bool isOpened() const;

    /// Положение рукоятки: от 0.0 (кран закрыт) до 1.0 (кран открыт)
    double getHandlePosition() const;

    /// Задать давление со стороны магистрали
    void setPipePressure(double value);

    /// Задать поток из оборудования
    void setDeviceFlow(double value);

    /// Давление со стороны оборудования
    double getPressureToDevice() const;

    /// Поток в магистраль
    double getFlowToPipe() const;

    enum {
        NUM_SOUNDS = 4,
        CHANGE_SOUND = 0,   ///< Звук переключения разобщительного крана
        OPEN_SOUND = 1,     ///< Звук открытия разобщительного крана
        CLOSE_SOUND = 2,    ///< Звук перекрытия разобщительного крана
        DRAIN_FLOW_SOUND = 3///< Звук выхода воздуха через атмосферное отверстие
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const override;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = CHANGE_SOUND) const override;

private:

    enum {
        HANDLE_POS = 0, ///< Y[0] - Положение рукоятки: от 0.0 (закрыт) до 1.0 (открыт)
        PRESSURE = 1    ///< Y[1] - Давление для оборудования за разобщительным краном
    };

    /// Время переключения концевого крана, с
    double switch_time = 0.2;

    /// Кран открыт
    bool is_opened = false;

    /// Давление в магистрали
    double p = 0.0;

    /// Поток из оборудования в магистраль
    double Q = 0.0;

    /// Поток выхода воздуха из оборудования через атмосферное отверстие
    double Q_atm = 0.0;

    /// Условный объём труб между краном и оборудованием
    double V0 = 1.0e-3;

    /// Коэффициент к потоку выхода воздуха из оборудования в атмосферу при закрытом кране
    double K_atm = 0.0;

    /// Коэффициент громкости озвучки к потоку разрядки
    double K_sound = 2.0;

    /// Заданное состояние крана: 0 - закрыт, 1 - открыт
    TriggerControl ref_state;

    /// Звук выхода воздуха из оборудования через атмосферное отверстие
    sound_state_t atm_flow_sound = sound_state_t();

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void stepKeysControl(double t, double dt) override;
};

#endif // PNEUMO_SHUTOFF_VALVE_H

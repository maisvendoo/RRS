#ifndef     PNEUMO_COMBINE_CRANE_H
#define     PNEUMO_COMBINE_CRANE_H

#include    "device.h"

class Timer;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoCombineCrane : public Device
{
public:

    PneumoCombineCrane(QObject *parent = nullptr);

    ~PneumoCombineCrane();

    /// Задать управляющую клавишу для переключения по часовой стрелке
    void setKeySymbolCombineCraneClockwise(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для переключения по часовой стрелке
    void setKeyModifierCombineCraneClockwise(std::uint16_t key_modifier);

    /// Задать управляющую клавишу для переключения в предыдущую позицию
    void setKeySymbolCombineCraneCounterclockwise(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для переключения в предыдущую позицию
    void setKeyModifierCombineCraneCounterclockwise(std::uint16_t key_modifier);

    void setControl(std::set<uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    /// Задать позицию комбинированного крана:
    /// -1 - положение двойной тяги, 0 - поездное положение, 1 - экстренное торможение
    void setCombineCranePosition(int pos);

    /// Положение рукоятки комбинированного крана:
    /// -1.0 - положение двойной тяги, 0.0 - поездное положение, 1.0 - экстренное торможение
    double getCombineCraneHandlePosition() const;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Давление тормозной магистрали к крану
    double getCraneBPpressure() const;

    /// Задать поток из крана в тормозную магистраль
    void setCraneBPflow(double value);

    /// Поток в тормозную магистраль
    double getBPflow() const;

    enum {
        CHANGE_COMB_POS_SOUND = 0,  ///< Звук переключения комбинированного крана
        BP_DRAIN_FLOW_SOUND = 1     ///< Звук опорожнения тормозной магистрали
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_COMB_POS_SOUND) const override;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_COMB_POS_SOUND) const override;

private:

    enum {
        HANDLE_POS = 0, ///< Y[0] - Положение рукоятки: от 0.0 (закрыт) до 1.0 (открыт)
        PRESSURE = 1,   ///< Y[1] - Давление для оборудования за комбинированным краном

        NUM_POSITIONS = 3   ///< Количество положений у комбинированного крана
    };

    /// Время переключения комбинированного крана, с
    double switch_time = 0.2;

    /// Давление от тормозной магистрали
    double pBP;

    /// Поток в тормозную магистраль
    double QBP;

    /// Поток из оборудования
    double Q;

    /// Условный объём труб между комбинированным краном и оборудованием за ним
    double  V0;

    /// Коэффициент потока - разрядки ТМ при экстренном торможении
    double K_emergency;

    /// Коэффициент громкости озвучки к потоку разрядки ТМ комбинированным краном
    double K_sound;

    /// Состояние комбинированного крана
    SwitcherControl ref_state = SwitcherControl(NUM_POSITIONS);

    /// Звук выхода воздуха из ТМ при экстренном торможении
    sound_state_t emergency_flow_sound = sound_state_t();

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void stepKeysControl(double t, double dt) override;
};

#endif // PNEUMO_COMBINE_CRANE_H

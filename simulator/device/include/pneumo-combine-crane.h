#ifndef     PNEUMO_COMBINE_CRANE_H
#define     PNEUMO_COMBINE_CRANE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoCombineCrane : public BrakeDevice
{
public:

    PneumoCombineCrane(QObject *parent = nullptr);

    ~PneumoCombineCrane();

    void init(double pBP, double pFL) override;

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

protected:

    enum {
        HANDLE_POS = 0, ///< Y[0] - Положение рукоятки: от -1.0 до 1.0
        PRESSURE_BP = 1,///< Y[1] - Давление для оборудования за комбинированным краном

        NUM_POSITIONS = 3   ///< Количество положений у комбинированного крана
    };

    /// Время переключения комбинированного крана, с
    double switch_time = 0.2;

    /// Давление от тормозной магистрали
    double pBP = 0.0;

    /// Поток в тормозную магистраль
    double QBP = 0.0;

    /// Поток из оборудования к тормозной магистрали
    double QBPcrane = 0.0;

    /// Условный объём труб между комбинированным краном и оборудованием за ним
    double  V0 = 1.0e-3;

    /// Коэффициент потока - разрядки ТМ при экстренном торможении
    double K_emergency = 0.1;

    /// Коэффициент громкости озвучки к потоку разрядки ТМ комбинированным краном
    double K_sound = 3.0;

    /// Требуемое состояние комбинированного крана
    SwitcherControl ref_state = SwitcherControl(NUM_POSITIONS);

    /// Звук выхода воздуха из ТМ при экстренном торможении
    sound_state_t emergency_flow_sound = sound_state_t();

    virtual void preStep(state_vector_t &Y, double t) override;

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    virtual void load_config(CfgReader &cfg) override;

    virtual void stepKeysControl(double t, double dt) override;
};

#endif // PNEUMO_COMBINE_CRANE_H

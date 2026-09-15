#ifndef     EP1M_TUMBLERS_PANEL_H
#define     EP1M_TUMBLERS_PANEL_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP1MTumblersPanel final : public Device
{
public:

    enum
    {
        TUMBLER_MSUD = 0,
        TUMBLER_LOCK_VVK = 1,
        TUMBLER_PANT1 = 2,
        TUMBLER_PANT2 = 3,
        TUMBLER_RETURN_PROTECTION = 4,
        TUMBLER_MAIN_SWITCH = 5,

        TUMBLER_AUX_MACHINES = 6,
        TUMBLER_COMPRESSOR = 7,
        TUMBLER_MOTOR_FAN1 = 8,
        TUMBLER_MOTOR_FAN2 = 9,
        TUMBLER_MOTOR_FAN3 = 10,
        TUMBLER_EPT = 11
    };

    EP1MTumblersPanel(QObject *parent = nullptr);

    ~EP1MTumblersPanel() override;

    void setControl(std::set<std::uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    void step(double t, double dt) override;

    /// Разрешить установить ключ (для реализации одного ключа на несколько кабин)
    void allowKey(bool allow);

    /// Разрешение установить ключ (для реализации одного ключа на несколько кабин)
    bool isKeyAllowed() const;

    /// Вставить/извлечь ключ
    void insertKey(bool insert);

    /// Признак установленного ключа
    bool isKey() const;

    /// Переключить ключ: false - заблокировать тумблеры, true - разблокировать
    void setKeyOn(bool state);

    /// Состояние ключа: false - тумблеры заблокированы, true - разблокированы
    bool isKeyOn() const;

    /// Задать состояние тумблера
    void setTumblerState(size_t tumbler_idx, bool state);

    /// Состояние тумблера
    bool getTumblerState(size_t tumbler_idx) const;

    float getKeyInsertSoundSignal(size_t idx = Trigger::CHANGE_SOUND);

    float getKeyTurnSoundSignal(size_t idx = Trigger::CHANGE_SOUND);

    float getTumblerSoundSignal(size_t tumbler_idx, size_t idx = Trigger::CHANGE_SOUND);

    TriggerControl *getTumblerPtr(size_t tumbler_idx)
    {
        return &tumblers[tumbler_idx];
    }

private:

    enum { TUMBLERS_NUMBER = 12 };

    /// Управляющая клавиша ключа
    std::uint16_t key_symbol = KEY_Undefined;

    /// Предыдущее состояние управляющей клавиши
    bool prev_key = false;

    /// Разрешение установить ключ (для реализации одного ключа на несколько кабин)
    bool is_key_allowed = true;

    /// Признак установленного ключа
    Trigger is_key = Trigger();

    /// Состояние ключа: false - тумблеры заблокированы, true - разблокированы
    Trigger key_state = Trigger();

    /// Тумблеры
    std::array<TriggerControl, TUMBLERS_NUMBER> tumblers;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void initControl();

    bool isAllTumblersOff() const;
};

#endif // EP1M_TUMBLERS_PANEL_H

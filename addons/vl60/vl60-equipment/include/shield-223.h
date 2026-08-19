#ifndef     SHIELD_223_H
#define     SHIELD_223_H

#include    "device.h"

//------------------------------------------------------------------------------
// Щиток 223/224 - дальний ряд тумблеров приборной панели машиниста
//------------------------------------------------------------------------------
class Shield_223 final : public Device
{
public:

    enum
    {
        /// Прожектор ярко
        SPOTLIGHT_HIGH = 0,
        /// Прожектор тускло
        SPOTLIGHT_LOW = 1,
        /// Радиостанция
        RADIO = 2,
        /// Цепи управления
        CIRCUIT = 3,
        /// Токоприемник задний
        PANT_BWD = 4,
        /// Токоприемник передний
        PANT_FWD = 5,
        /// Токоприемники
        PANTS = 6,
        /// Включение ГВ и возврат защиты
        RETURN_PROTECTION = 7,
        /// Выключатель ГВ
        MAIN_SWITCH = 8,

        TUMBLERS_NUMBER = 9
    };

    Shield_223(QObject* parent = nullptr);

    ~Shield_223() = default;

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

private:

    static constexpr std::uint8_t locked_tumblers[] = {RADIO, CIRCUIT, PANT_BWD, PANT_FWD, PANTS, RETURN_PROTECTION, MAIN_SWITCH};
    static constexpr std::uint8_t no_locked_tumblers[] = {SPOTLIGHT_HIGH, SPOTLIGHT_LOW};

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

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void initControl();

    bool isAllTumblersOff() const;
};

#endif // SHIELD_223_H

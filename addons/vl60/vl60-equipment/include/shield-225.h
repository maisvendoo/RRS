#ifndef     SHIELD_225_H
#define     SHIELD_225_H

#include    "device.h"

//------------------------------------------------------------------------------
// Щиток 225/226 - бллижний ряд тумблеров приборной панели машиниста
//------------------------------------------------------------------------------
class Shield_225 final : public Device
{
public:

    enum
    {
        /// Автоматическая подача песка
        AUTOSAND = 0,
        /// Вентилятор 6
        FAN_6 = 1,
        /// Вентилятор 5
        FAN_5 = 2,
        /// Вентилятор 4
        FAN_4 = 3,
        /// Вентилятор 3
        FAN_3 = 4,
        /// Вентилятор 2
        FAN_2 = 5,
        /// Вентилятор 1
        FAN_1 = 6,
        /// Компрессор
        COMPRESSOR = 7,
        /// Фазорасщепитель
        PHASE_SPLITTER = 8,

        TUMBLERS_NUMBER = 9
    };

    Shield_225(QObject* parent = nullptr);

    ~Shield_225() = default;

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

    static constexpr std::uint8_t locked_tumblers[] = {AUTOSAND, FAN_6, FAN_5, FAN_4, FAN_3, FAN_2, FAN_1, COMPRESSOR, PHASE_SPLITTER};
//    static constexpr std::uint8_t no_locked_tumblers[] = {}; // Ключ блокирует всё

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

#endif // SHIELD_225_H

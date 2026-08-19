#ifndef     BRAKE_LOCK_H
#define     BRAKE_LOCK_H

#include    "pneumo-combine-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoBrakeLock final : public PneumoCombineCrane
{
public:

    PneumoBrakeLock(QObject *parent = nullptr);

    ~PneumoBrakeLock();

    void init(double pBP, double pFL) override;

    /// Задать управляющую клавишу включения/отключения блокировки
    void setKeySymbolChangeLockState(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор включения/отключения блокировки
    void setKeyModifierChangeLockState(std::uint16_t key_modifier);

    // Блокировка
    /// Разрешить установить рукоятку блокировки (для реализации одной рукоятки на несколько кабин)
    void allowLockHandle(bool allow);

    /// Разрешение установить рукоятку блокировки (для реализации одной рукоятки на несколько кабин)
    bool isLockHandleAllowed() const;

    /// Установить/извлечь рукоятку блокировки
    void insertLockHandle(bool insert);

    /// Признак установленной рукоятки блокировки
    bool isLockHandle() const;

    /// Задать состояние блокировки:
    /// false - отключить, true - включить;
    void setStateOn(bool state);

    /// Состояние блокировки:
    /// false - отключено, true - включено
    bool isStateOn() const;

    /// Состояние стопора рукоятки давлением в тормозной магистрали:
    /// false - вращение разблокировано, true - вращение заблокировано
    bool isStateLockedByBPpressure() const;

    /// Положение рукоятки блокировки:
    /// 0.0 - отключенное положение, 1.0 - включенное положение
    double getLockHandlePosition() const;

    // Взаимодействие оборудования, подключаемого через данную блокировку
    // Взаимодействие кранов и питательной магистрали
    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Давление питательной магистрали к крану
    double getCraneFLpressure() const;

    /// Задать поток из крана в питательную магистраль
    void setCraneFLflow(double value);

    /// Поток в питательную магистраль
    double getFLflow() const;

    // Взаимодействие кранов и магистрали тормозных цилиндров
    /// Задать давление от магистрали тормозных цилиндров
    void setBCpressure(double value);

    /// Давление магистрали тормозных цилиндров к крану
    double getCraneBCpressure() const;

    /// Задать поток из крана в магистраль тормозных цилиндров
    void setCraneBCflow(double value);

    /// Поток в магистраль тормозных цилиндров
    double getBCflow() const;

    enum {
        LOCK_HANDLE_CHANGED = 2 + Trigger::CHANGE_SOUND,///< Звук установки/извлечения рукоятки
        LOCK_HANDLE_INSERTED = 2 + Trigger::ON_SOUND,   ///< Звук установки рукоятки
        LOCK_HANDLE_REMOVED = 2 + Trigger::OFF_SOUND,   ///< Звук извлечения рукоятки
        LOCK_STATE_CHANGED = 5 + Trigger::CHANGE_SOUND, ///< Звук поворота рукоятки
        LOCK_STATE_ON = 5 + Trigger::ON_SOUND,          ///< Звук поворота во включенное положение
        LOCK_STATE_OFF = 5 + Trigger::OFF_SOUND         ///< Звук поворота в выключенное положение
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = CHANGE_COMB_POS_SOUND) const override;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = CHANGE_COMB_POS_SOUND) const override;

private:

    enum {
        /* PneumoCombineCrane
        HANDLE_POS = 0, ///< Y[0] - Положение рукоятки: от -1.0 до 1.0
        PRESSURE_BP = 1,///< Y[1] - Давление для оборудования за комбинированным краном
        */
        LOCK_POS = 2,   ///< Y[2] - Положение рукоятки блокировки: от 0.0 до 1.0
        PRESSURE_FL = 3,///< Y[3] - Давление питательной магистрали для оборудования за блокировкой
        PRESSURE_BC = 4 ///< Y[4] - Давление магистрали ТЦ для оборудования за блокировкой
    };

    /// Управляющая клавиша на включение/отключение блокировки
    std::uint16_t key_symbol_change_state = KEY_Undefined;

    /// Клавиша-модификатор на включение/отключение блокировки
    std::uint16_t key_modifier_change_state = KEY_Undefined;

    /// Предыдущее состояние управляющей клавиши
    bool prev_key = false;

    /// Разрешение установить рукоятку (для реализации одной рукоятки на несколько кабин)
    bool is_handle_allowed = true;

    /// Признак стопора рукоятки давлением в тормозной магистрали
    bool is_handle_locked = false;

    /// Требуемое состояние блокировки
    bool ref_lock_state = false;

    /// Время переключения блокировки, с
    double lock_switch_time = 0.2;

    /// Давление в тормозной магистрали, блокирующее рукоятку блокировки
    double p_lock = 0.1;

    /// Давление от питательной магистрали
    double pFL = 0.0;
    /// Давление от магистрали тормозных цилиндров
    double pBC = 0.0;

    /// Поток в питательную магистраль
    double QFL = 0.0;
    /// Поток в магистраль тормозных цилиндров
    double QBC = 0.0;

    /// Поток из оборудования к питательной магистрали
    double QFLcrane = 0.0;
    /// Поток из оборудования к магистрали тормозных цилиндров
    double QBCcrane = 0.0;

    /// Признак установленной рукоятки блокироки
    Trigger is_lock_handle = Trigger();

    /// Состояние блокировки: false - отключено, true - включено
    Trigger lock_state = Trigger();

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;

    void stepKeysControl(double t, double dt) override;
};

#endif // BRAKE_LOCK_H

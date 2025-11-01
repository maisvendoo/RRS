#ifndef     KME_60_044_H
#define     KME_60_044_H

#include    "device.h"
#include    "km-state.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControllerKME_60_044 : public Device
{
public:

    ControllerKME_60_044(QObject *parent = nullptr);

    ~ControllerKME_60_044();

    /// Разрешить установить реверсивку (для реализации одной рукоятки на несколько кабин)
    void allowReversHandle(bool allow);

    /// Разрешение установить реверсивку (для реализации одной рукоятки на несколько кабин)
    bool isReversHandleAllowed() const;

    /// Вставить/извлечь реверсивную рукоятку
    void insertReversHandle(bool insert);

    /// Признак вставленной реверсивной рукоятки
    bool isReversHandle() const;

    /// Задать положение реверсивной рукоятки
    void setReversHandlePos(int pos);

    /// Положение реверсивной рукоятки
    float getReversHandlePos() const;

    /// Задать положение главной рукоятки
    void setMainHandlePos(int pos);

    /// Положение главной рукоятки
    float getMainHandlePos() const;

    QString getMainHandlePosName() const;

    km_state_t getState() const;

    enum {
        NUM_SOUNDS = 2,
        REVERS_CHANGE_POS_SOUND = 0,    ///< Звук переключения реверсора
        MAIN_CHANGE_POS_SOUND = 1,      ///< Звук переключения контроллера
        HANDLE_CHANGE_SOUND = NUM_SOUNDS + Trigger::CHANGE_SOUND,
        HANDLE_INSERTED_SOUND = NUM_SOUNDS + Trigger::ON_SOUND,
        HANDLE_REMOVED_SOUND = NUM_SOUNDS + Trigger::OFF_SOUND,
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = REVERS_CHANGE_POS_SOUND) const;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = REVERS_CHANGE_POS_SOUND) const;

private:

    bool is_prev_KEY_D = false;
    bool is_prev_KEY_W = false;
    bool is_prev_KEY_S = false;

    /// Разрешение установить реверсивку (для реализации одной рукоятки на несколько кабин)
    bool is_reverse_handle_allowed = true;

    /// Состояние главного вала
    std::uint8_t main_pos = POS_ZERO;
    /// Состояние реверсивного вала
    std::uint8_t revers_pos = REVERS_ZERO;

    /// Состояние контроллера
    km_state_t  state;

    /// Положение главной рукоятки
    float       main_handle_pos = 0.0f;
    /// Положение реверсивной рукоятки
    float       revers_handle_pos = 0.0f;

    /// Признак реверсивной рукоятки
    Trigger is_revers_handle = Trigger();

    /// Счётчик и состояние звуков
    std::array <sound_state_t, NUM_SOUNDS> sounds = {sound_state_t(), sound_state_t()};

    enum
    {
        SWITCH_TIMEOUT = 300
    };

    Timer   incMainPos = Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    Timer   decMainPos = Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);

    Timer   incReversPos = Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);
    Timer   decReversPos = Timer(static_cast<double>(SWITCH_TIMEOUT) / 1000.0);

    QStringList positions_names = {"BV", " 0", "AV", "RV", "FV", "FP", "RP", "AP"};

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void soundMainChangePos();
    void soundReversChangePos();

private slots:

    void incMain();

    void decMain();

    void incRevers();

    void decRevers();
};

#endif // KME_60_044_H

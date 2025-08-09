#ifndef     KME_60_044_H
#define     KME_60_044_H

#include    "traction-controller.h"
#include    "km-state.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControllerKME_60_044 : public TractionController
{
public:

    ControllerKME_60_044(QObject *parent = nullptr);

    ~ControllerKME_60_044();

    /// Вставить/извлечь реверсивную рукоятку
    void insertReversHandle(bool insert);

    /// Задать положение главной рукоятки
    void setMainHandlePos(int pos);

    /// Задать положение реверсивной рукоятки
    void setReversHandlePos(int pos);

    /// Признак вставленной реверсивной рукоятки
    bool isReversHandle() const;

    km_state_t getState() const;

    QString getMainHandlePosName() const;

    float getMainHandlePos() const;

    float getReversHandlePos() const;

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

    /// Состояние главного вала
    std::uint8_t main_pos;
    /// Состояние реверсивного вала
    std::uint8_t revers_pos;

    /// Признак реверсивной рукоятки
    Trigger is_revers_handle;

    /// Положение главной рукоятки
    float       main_handle_pos;

    /// Положение реверсивной рукоятки
    float       revers_handle_pos;

    /// Состояние контроллера
    km_state_t  state;

    /// Счётчик и состояние звуков
    std::array <sound_state_t, NUM_SOUNDS> sounds;

    enum
    {
        SWITCH_TIMEOUT = 300
    };

    Timer       *incMainPos;
    Timer       *decMainPos;

    Timer       *incReversPos;
    Timer       *decReversPos;

    bool is_prev_KEY_D = false;
    bool is_prev_KEY_W = false;
    bool is_prev_KEY_S = false;
    QStringList positions_names;

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

#ifndef     EKG_8G_H
#define     EKG_8G_H

#include    "device.h"
#include    "timer.h"
#include    "km-state.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EKG_8G : public Device
{
public:

    EKG_8G(QObject *parent = Q_NULLPTR);

    ~EKG_8G();

    void setKMstate(const km_state_t &km_state);

    float getSelsinPosition() const;

    bool isLongMotionPos() const;

    void enable(bool value);

    int getPosition() const;

    bool isReady() const;

    bool isLKallow() const;

    enum {
        NUM_SOUNDS = 4,
        CHANGE_POS_ONE_1 = 0,
        CHANGE_POS_ONE_2 = 1,
        CHANGE_POS_AUTO_1 = 2,
        CHANGE_POS_AUTO_2 = 3,

        NO_SOUND = 0,
        SOUND_ONE = 1,
        SOUND_AUTO = 2
    };
    sound_state_t getSound(size_t idx);

private:

    enum
    {
        NUM_POSITIONS = 38,
        PP_MIN = 18,
        PP_MAX = 22
    };

    /// Номер текущей позиции (с учетом переходных позиций)
    int  position;

    /// Номер заданной позиции
    int ref_position;

    /// Время переключения одной позиции, с
    double  switch_time;

    /// Признак подачи управляющего питания на серводвигатель ЭКГ
    bool    is_enabled;

    /// Признак готовности ЭКГ к работе
    bool    is_ready;

    /// Признак разрешения на включение линейных контакторов
    bool    is_LK_allow;

    bool    is_fix_start;

    bool    is_fix_off;

    int     dir;

    bool    is_auto;

    /// Таймер управления переключением позиций
    Timer   pos_switcher;

    /// Состояние контроллера машиниста
    km_state_t  km_state;

    /// Тригер для фиксации пуска
    Trigger      fix_start;

    /// Тригер для фиксации выключения
    Trigger      fix_off;

    /// Признаки нулевой и ходовых позиций
    std::array<bool, NUM_POSITIONS> is_long_motion;

    enum
    {
        LM_POS0 = 0,
        LM_POS1 = 5,
        LM_POS2 = 9,
        LM_POS3 = 13,
        LM_POS4 = 17,
        LM_POS5 = 25,
        LM_POS6 = 29,
        LM_POS7 = 33,
        LM_POS8 = 37
    };

    /// Очерёдность и состояние звуков
    std::array <sound_state_t, 4> sounds;
    char is_sound_one_or_auto;
    bool is_change_one_1_or_2;
    bool is_change_auto_1_or_2;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepDiscrete(double t, double dt);

    void process();

private slots:

    void slotPosSwitch();
};

#endif // EKG_8G_H

#ifndef     KM_35_01_H
#define     KM_35_01_H

#include    "device.h"

//------------------------------------------------------------------------------
//  Контроллер машиниста электровоза ЭП1м(п)
//------------------------------------------------------------------------------
class TracController : public Device
{
public:

    TracController(QObject *parent = Q_NULLPTR);

    ~TracController();

    /// Разрешить установить реверсивку (для реализации одной рукоятки на несколько кабин)
    void allowReversHandle(bool allow);

    /// Разрешение установить реверсивку (для реализации одной рукоятки на несколько кабин)
    bool isReversHandleAllowed() const;

    /// Вставить/извлечь реверсивную рукоятку
    void insertReversHandle(bool insert);

    /// Признак вставленной реверсивной рукоятки
    bool isReversHandle() const;

    bool isZero() const
    {
        return mode_pos == 0;
    }

    bool isTraction() const
    {
        return mode_pos == 1;
    }

    bool isBrake()
    {
        return mode_pos == -1;
    }

    bool isContacts15_16 () const
    {
        return (trac_level == 0) && (brake_level == 0);
    }

    bool isContacts5_6() const
    {
        return (revers_pos == 1) || (revers_pos == -1);
    }

    bool isContacts7_8() const
    {
        return (revers_pos == 1) || (revers_pos == -1);
    }

    bool isContacts9_10() const
    {
        return mode_pos == 1;
    }

    bool isContacts11_12() const
    {
        return mode_pos == -1;
    }

    bool isContacts3_4() const
    {
        return revers_pos == -1;
    }

    bool isContacts1_2() const
    {
        return revers_pos == 1;
    }

    bool isContacts13_14() const
    {
        return (mode_pos == 1) || (mode_pos == -1);
    }

    float getReversHandlePos() const { return static_cast<float>(revers_pos); }

    float getHandlePosition() const;

    double getTracLevel() const { return static_cast<double>(trac_level) / 100.0; }

    double getBrakeLevel() const { return static_cast<double>(brake_level) / 100.0; }

    double getRefSpeedLevel() const { return refV_level; }

    enum {
        NUM_SOUNDS = 2,
        REVERS_CHANGE_POS_SOUND = 0,    ///< Звук переключения реверсора
        MAIN_CHANGE_POS_SOUND = 1,      ///< Звук переключения контроллера
        HANDLE_CHANGE_SOUND = NUM_SOUNDS + Trigger::CHANGE_SOUND,
        HANDLE_INSERTED_SOUND = NUM_SOUNDS + Trigger::ON_SOUND,
        HANDLE_REMOVED_SOUND = NUM_SOUNDS + Trigger::OFF_SOUND,
    };
    float getSoundSignal(size_t state_idx) const override;

    void setReversFwd()
    {
        revers_pos = 1;

        sound_states[REVERS_CHANGE_POS_SOUND].play();
    }

    void setReversZero()
    {
        revers_pos = 0;

        sound_states[REVERS_CHANGE_POS_SOUND].play();
    }

    void setReversBwd()
    {
        revers_pos = -1;

        sound_states[REVERS_CHANGE_POS_SOUND].play();
    }

    void setMode(int8_t mode_pos)
    {
        this->mode_pos = mode_pos;
    }

    void setLevel(double level)
    {
        if (mode_pos == 1 && level >= 0)
        {
            trac_level = cut(level, 0.0, 1.0) * 100.0;
        }
        else
        {
            trac_level = 0.0;
        }

        if (mode_pos == -1 && level <= 0)
        {
            brake_level = cut(-level, 0.0, 1.0) * 100.0;
        }
        else
        {
            brake_level = 0.0;
        }
    }

    void setVrefLevel(double v_level)
    {
        refV_level = cut(v_level, 0.0, 1.0);
    }

private:

    /// Разрешение установить реверсивку (для реализации одной рукоятки на несколько кабин)
    bool is_reverse_handle_allowed = true;

    /// Позиция, определяющая состояние схемы
    /// (0 - схема разобрана, 1 - подготовка тяги, 2 - подготовка рекуперации)
    std::int8_t mode_pos = 0;
    std::int8_t mode_pos_old = 0;

    std::int8_t revers_pos = 0;

    bool old_fwd_key_state = false;

    bool old_bwd_key_state = false;

    bool old_traction_key = false;

    bool old_brake_key = false;

    int trac_level = 0;

    int brake_level = 0;

    /// Вращение контроллера по сигналу таймера
    int handle_motion_speed = 0;

    /// Коэффициент ускорения контроллера с нажатым Shift
    int handle_high_speed_coeff = 8;

    /// Положение регулятора скорости
    double refV_level = 0.0;

    /// Шаг вращения регулятора скорости
    double refV_step = 1.0 / 140.0;

    /// Вращение регулятора скорости по сигналу таймера
    double refV_motion_speed = 0.0;

    /// Коэффициент ускорения регулятора скорости с нажатым Shift
    double refV_high_speed_coeff = 8.0;

    Timer tracTimer;

    Timer brakeTimer;

    Timer speedTimer;

    /// Признак реверсивной рукоятки
    Trigger is_revers_handle;

    Trigger traction;

    Trigger brake;

    std::array<sound_state_t, NUM_SOUNDS> sound_states;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void load_config(CfgReader &cfg) override;

    void stepKeysControl(double t, double dt) override;

    void processDiscretePositions(bool key_state, bool old_key_state, int dir);

private slots:

    void slotTracLevelProcess();

    void slotBrakeLevelProcess();

    void slotSpeedLevelProcess();
};

#endif // KM_35_01_H

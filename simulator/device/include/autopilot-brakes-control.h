#ifndef     AUTOPILOT_BRAKE_CONTROL_H
#define     AUTOPILOT_BRAKE_CONTROL_H

#include    <device.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    KRM_POS_I = 0,
    KRM_POS_II = 1,
    KRM_POS_III = 2,
    KRM_POS_IV = 3,
    KRM_POS_Va = 4,
    KRM_POS_V = 5,
    KRM_POS_VI = 6
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct autopilot_brake_control_state_t
{
    /// Заданная позиция крана машиниста
    int brake_crane_pos_ref = KRM_POS_II;
    /// Заданное положение крана вспомогательного тормоза
    double loco_crane_pos_ref = 1.0;

    autopilot_brake_control_state_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AutopilotBrakeController : public Device
{
    Q_OBJECT

public:

    AutopilotBrakeController();

    ~AutopilotBrakeController() = default;

    /// Задать давления в тормозной системе
    void setBrakePressures(double pEQ, double pBC, double p_charge)
    {
        this->pEQ = pEQ;
        this->pBC = pBC;
        this->p_charge = p_charge;
    }

    void step(double t, double dt) override;

    /// Шаг управления с учетом всех признаков
    void step_control(bool is_EPB_ON,
                      double dv,
                      bool is_motion_allowed,
                      bool &lock_traction,
                      bool &is_disable_release);

    /// Вернуть состояние органов управления
    autopilot_brake_control_state_t getControlState() const
    {
        return bc_state;
    }

    void setFeedback(double v_cur, double dist_target, double a_ref, double a_cur)
    {
        this->v_cur = v_cur;
        this->dist_target = dist_target;
        this->a_ref = a_ref;
        this->a_cur = a_cur;
    }

signals:

    void sigSetBrakeCurveAccel(double a_brake);

private:

    /// Состояние органов управления тормозами
    autopilot_brake_control_state_t bc_state;

    /// Давление в УР
    double pEQ = 0.0;
    /// Давление в ТЦ
    double pBC = 0.0;
    /// Зарядное давление
    double p_charge = 0.0;

    /// Текущаяя скорость
    double v_cur = 0.0;

    /// Дистанция до цели
    double dist_target = 0.0;

    /// Число дополнительных ступеней разрядки ТМ
    uint8_t num_steps = 0;

    /// Текущее ускорение поезда
    double a_cur = 0.0;

    /// Ускорение поезда, задаваемое кривой снижения скорости
    double a_ref = 0.0;

    /// Первышение скорости над программной для торможения ЭПТ
    double dVminusEPB = -0.5;

    /// Снижение скорости при отпуске ЭПТ
    double dVplusEPB = 3.0;

    /// Первышение скорости над программной для торможения ПТ
    double dVminusPB = -0.5;

    /// Снижение скорости при отпуске ПТ
    double dVplusPB = 3.0;

    /// Завышение зарядного давления при отпуске ЭПТ
    double dpEPB_over = 0.02;

    /// Завышение зарядного давления при отпуске ПТ
    double dpPB_over = 0.0;

    /// Величина минимальной ступени ПТ
    double dp_first_step = 0.04;

    /// Величина каждой последующей ступени ПТ
    double dp_other_step = 0.02;

    /// Время выдержки в перекрыше для оценки эффективности ступени при ПТ
    double hold_timeout = 2.0;

    /// Минимальная ступень ЭПТ
    double pBC_EPB = 0.1;

    const double KRM_HANDLE_DELAY = 0.5;
    Timer *krm_handle_timer = new Timer(KRM_HANDLE_DELAY, false);

    const double BRAKE_DELAY = 2.0;
    Timer *brake_timer = new Timer(BRAKE_DELAY, false);

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override
    {
        (void) Y; (void) dYdt; (void) t;
    }

    void load_config(CfgReader &cfg) override;

    /// Управление ЭПТ
    void stepEPB(double dv, bool &lock_traction, bool &is_disable_release);

    /// Управление ПТ
    void stepPB(double dv, bool is_motion_allowed, bool &lock_traction, bool &is_disable_release);

    /// Управление КВТ
    void stepKVT(bool is_motion_allowed, bool &is_disable_release);

    /// Перевести кран в заданное положение с выдержкой в нем
    void setBrakeCranePos(int pos);

    /// Выполнить ступень торможения ПТ
    void brakeStep(double pEQ, double p_charge, double dp);

    /// Выполнить отпуск ПТ
    void brakeRelease(double pEQ, double p_charge, double dp_over);

private slots:

    void slotBrakeCraneHandle();

    void slotBrakeDelay();
};

#endif

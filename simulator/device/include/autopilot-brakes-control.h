#ifndef     AUTOPILOT_BRAKE_CONTROL_H
#define     AUTOPILOT_BRAKE_CONTROL_H

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
class AutopilotBrakeController
{
public:

    AutopilotBrakeController() = default;

    ~AutopilotBrakeController() = default;

    /// Задать давления в тормозной системе
    void setBrakePressures(double pEQ, double pBC, double p_charge)
    {
        this->pEQ = pEQ;
        this->pBC = pBC;
        this->p_charge = p_charge;
    }

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

private:

    /// Состояние органов управления тормозами
    autopilot_brake_control_state_t bc_state;

    /// Давление в УР
    double pEQ = 0.0;
    /// Давление в ТЦ
    double pBC = 0.0;
    /// Зарядное давление
    double p_charge = 0.0;

    /// Управление ЭПТ
    void stepEPB(double dv, bool &lock_traction, bool &is_disable_release);

    /// Управление ПТ
    void stepPB(double dv, bool &lock_traction, bool &is_disable_release);

    /// Управление КВТ
    void stepKVT(bool is_motion_allowed, bool &is_disable_release);
};

#endif

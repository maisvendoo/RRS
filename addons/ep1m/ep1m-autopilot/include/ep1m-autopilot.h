#ifndef     EP1M_AUTOPILOT
#define     EP1M_AUTOPILOT

#include    <autopilot.h>
#include    <timer.h>
#include    <ep1m-autopilot-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP1mAutopilot : public Autopilot
{
public:

    EP1mAutopilot();

    ~EP1mAutopilot();

    auto_control_t *getControl() override;

    void step(double t, double dt) override;

    void initAutoBrakeControl(const QString& config_name,
                              const QString& custom_cfg_dir) override;

private:

    /// структура управляющих воздействий
    ep1m_control_t *auto_control = new ep1m_control_t();

    /// структура обратных связей
    ep1m_feedback_t *auto_feedback = nullptr;

    /// Максимальный ток якоря
    double Imax = 0.0;

    /// Ошибка по скорости
    double dv = 0.0;

    /// Коэффициент пропорциональной части регулятора
    double Kp = 1.0;

    /// Коэффициент обратной связи по скорости проскальзывания
    double Ks = 1.0;

    int8_t mode_pos = 0;
    int8_t mode_pos_old = 0;

    /// Максимальный ток, задаваемый с контроллера
    double I_ref_max = 1300.0;

    bool lock_traction = false;

    bool edb_disable = false;

    /// Тормозной контроллер
    AutopilotBrakeController *brake_control = new AutopilotBrakeController;

    /// Таймер выдержки для КМ
    Timer *km_delay = new Timer(1.5, false);

    void press_RB() override;

    void release_RB() override;

    void load_config(CfgReader &cfg) override;

    void preStep(state_vector_t &Y, double t) override;

    /// Трехпозиционное реле с зоной нечувствительности
    int8_t tree_pos_relay(double x, double x_min, double x_max);

    /// Управление тягой/ЭДТ
    void traction_control(int8_t mode_pos, double level);

private slots:

    void slotDelayKM();
};

#endif

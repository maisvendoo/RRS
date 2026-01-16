#ifndef     VL60_AUTOPILOT
#define     VL60_AUTOPILOT

#include    <autopilot.h>
#include    <vl60-autopilot-types.h>
#include    <timer.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60Autopilot : public Autopilot
{
public:

    VL60Autopilot();

    ~VL60Autopilot();

    auto_control_t *getControl() override;    

    void step(double t, double dt) override;

private:

    /// Ограничение тока якоря
    double Imax = 500.0;
    /// Величина падения тока якоря для набора следующий позиции
    double delta_I = 50.0;

    /// Предыдущая тяговая позиция
    int prev_pos = 0;

    /// Выдержка рукоятки КМ
    const double KM_DELAY = 0.5;

    /// Таймер выдержки главной рукоятки КМ в заданном
    Timer *delay = new Timer(KM_DELAY, false);

    /// Коэффициент пропорциональной части регулятора скорости
    double Kp = 0.04;
    /// Коэффициент интегральной части регулятора скорости
    double Ki = 0.00001;

    /// Ошибка по скорости
    double dv = 0.0;

    /// Структура управляющих воздействий ВСЕГДА специфична
    vl60_control_t *auto_control = new vl60_control_t();

    /// Структура обратной связи, приводимая к нашей от общей
    vl60_feedback_t *auto_feedback = nullptr;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void load_config(CfgReader &cfg) override;    

    void release_RB() override;

    void press_RB() override;

    /// Набор одной позиции
    void plusPos();

    /// Сброс одной позиции
    void minusPos();

    /// Ступень торможения
    void brakeStep(double p_charge, double dp);

private slots:

    void slotDelayTimer();
};

#endif

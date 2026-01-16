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

    void setFeedback(auto_feedback_t *feedback) override;

    void step(double t, double dt) override;

private:

    /// Ограничение тока якоря
    double Imax = 500.0;
    /// Величина падения тока якоря для набора следующий позиции
    double delta_I = 50.0;

    /// Предыдущая тяговая позиция
    int prev_pos = 0;

    /// Таймер выдержки КМ в промежуточном положении
    Timer *delay = new Timer(0.5, false);

    vl60_control_t *auto_control = new vl60_control_t();

    vl60_feedback_t *auto_feedback = nullptr;

    void load_config(CfgReader &cfg) override;

    void vigilance_control(double t, double dt) override;

    void onPressRB_Timeout() override;

    /// Набор одной позиции
    void plusPos();

    /// Сброс одной позиции
    void minusPos();

private slots:

    void slotDelayTimer();
};

#endif

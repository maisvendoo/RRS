#ifndef     VL60_AUTOPILOT
#define     VL60_AUTOPILOT

#include    <autopilot.h>
#include    <vl60-autopilot-types.h>

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

private:

    vl60_control_t *auto_control = new vl60_control_t();

    vl60_feedback_t *auto_feedback = nullptr;
};

#endif

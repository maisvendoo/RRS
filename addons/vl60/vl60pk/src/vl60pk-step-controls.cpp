#include    <vl60pk.h>
#include    <vl60-controls.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepControls(const double &t, const double &dt)
{
    for (auto cab_idx : {CAB1, CAB2})
    {
        bool is_pnt = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_PNT]);

        if (is_pnt)
        {
            pants_tumbler[cab_idx].set();
        }
        else
        {
            pants_tumbler[cab_idx].reset();
        }

        bool is_pnt1 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_PNT1]);

        if (is_pnt1)
        {
            pant1_tumbler[cab_idx].set();
        }
        else
        {
            pant1_tumbler[cab_idx].reset();
        }

        bool is_pnt2 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_PNT2]);

        if (is_pnt2)
        {
            pant2_tumbler[cab_idx].set();
        }
        else
        {
            pant2_tumbler[cab_idx].reset();
        }
    }
}

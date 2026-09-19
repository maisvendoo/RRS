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
        is_pnt ? pants_tumbler[cab_idx].set() : pants_tumbler[cab_idx].reset();

        bool is_pnt1 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_PNT1]);
        is_pnt1 ? pant1_tumbler[cab_idx].set() : pant1_tumbler[cab_idx].reset();

        bool is_pnt2 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_PNT2]);
        is_pnt2 ? pant2_tumbler[cab_idx].set() : pant2_tumbler[cab_idx].reset();

        bool is_main_switch_on = static_cast<bool>(control_inputs[cab_idx][CTRL_MAIN_SWITCH_ON]);
        is_main_switch_on ? gv_tumbler[cab_idx].set() : gv_tumbler[cab_idx].reset();

        bool is_main_switch_return = static_cast<bool>(control_inputs[cab_idx][CTRL_RETURN_PROTECTION]);
        is_main_switch_return ? gv_return_tumbler[cab_idx].set() : gv_return_tumbler[cab_idx].reset();

        bool is_fr = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_FR]);
        is_fr ? fr_tumbler[cab_idx].set() : fr_tumbler[cab_idx].reset();

        bool is_mk = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MK]);
        is_mk ? mk_tumbler[cab_idx].set() : mk_tumbler[cab_idx].reset();
    }
}

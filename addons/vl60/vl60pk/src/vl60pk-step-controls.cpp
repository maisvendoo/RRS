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

        bool is_mv1 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV1]);
        is_mv1 ? mv_tumblers[cab_idx][MV1].set() : mv_tumblers[cab_idx][MV1].reset();

        bool is_mv2 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV2]);
        is_mv2 ? mv_tumblers[cab_idx][MV2].set() : mv_tumblers[cab_idx][MV2].reset();

        bool is_mv3 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV3]);
        is_mv3 ? mv_tumblers[cab_idx][MV3].set() : mv_tumblers[cab_idx][MV3].reset();

        bool is_mv4 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV4]);
        is_mv4 ? mv_tumblers[cab_idx][MV4].set() : mv_tumblers[cab_idx][MV4].reset();

        bool is_mv5 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV5]);
        is_mv5 ? mv_tumblers[cab_idx][MV5].set() : mv_tumblers[cab_idx][MV5].reset();

        bool is_mv6 = static_cast<bool>(control_inputs[cab_idx][CTRL_TUMBLER_MV6]);
        is_mv6 ? mv_tumblers[cab_idx][MV6].set() : mv_tumblers[cab_idx][MV6].reset();
    }
}

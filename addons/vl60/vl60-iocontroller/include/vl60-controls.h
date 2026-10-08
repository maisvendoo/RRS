#ifndef     VL60_CONTROLS_H
#define     VL60_CONTROLS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    CTRL_TUMBLER_PNT = 100,
    CTRL_TUMBLER_PNT1 = 101,
    CTRL_TUMBLER_PNT2 = 102,
    CTRL_MAIN_SWITCH_ON = 103,
    CTRL_RETURN_PROTECTION = 104,
    CTRL_TUMBLER_FR = 105,
    CTRL_TUMBLER_MK = 106,
    CTRL_TUMBLER_MV1 = 107,
    CTRL_TUMBLER_MV2 = 108,
    CTRL_TUMBLER_MV3 = 109,
    CTRL_TUMBLER_MV4 = 110,
    CTRL_TUMBLER_MV5 = 111,
    CTRL_TUMBLER_MV6 = 112,
    CTRL_TUMBLER_CU = 113,
    CTRL_TUMBLER_SPOT_HIGH = 114,
    CTRL_TUMBLER_SPOT_LOW = 115,
    CTRL_TUMBLER_CAB_LIGHT_LOW = 116,
    CTRL_TUMBLER_CAB_LIGHT_HIGH = 117,
    CTRL_TUMBLER_LIGHT_DEVICES = 118,
    CTRL_TUMBLER_BUF_LIGHT_L = 119,
    CTRL_TUMBLER_BUF_LIGHT_R = 120,
    CTRL_TUMBLER_BUF_COLOR_L = 121,
    CTRL_TUMBLER_BUF_COLOR_R = 122,
    CTRL_TUMBLER_EPB = 123,
    CTRL_REVERS_INSERTION = 124,
    CTRL_EPK_INSERTION = 125,
    CTRL_LOCK_367_INSERTION = 126,
    CTRL_KEY_EPK = 127,
    CTRL_REVERS_POSITION = 128,
    CTRL_KM_MAIN_POSITION = 129,
    CTRL_KRM_395 = 130,
    CTRL_KVT_254 = 131,
    CTRL_COMBINE_KRAN = 132,
    CTRL_SAND_BUTTON = 133,
    CTRL_WHISTLE_BUTTON = 134,
    CTRL_TIFON_BUTTON = 135,
    CTRL_RBS_BUTTON = 136,
    CTRL_RB_BUTTON = 137,
    CTRL_RBP_BUTTON = 138,
    CTRL_AUTOPILOT = 139,
    CTRL_SHUNTING_MODE = 140,

    // Концевые краны тормозной магистрали
    CTRL_ANGLECOCK_BP_FWD = 141,
    CTRL_ANGLECOCK_BP_BWD = 142,

    // Концевые краны питательной магистрали
    CTRL_ANGLECOCK_FL_FWD = 143,
    CTRL_ANGLECOCK_FL_BWD = 144,

    // Концевые краны тормозных цилиндров
    CTRL_ANGLECOCK_BC_FWD = 145,
    CTRL_ANGLECOCK_BC_BWD = 146,

    // Автозапуск и автоостанов
    CTRL_AUTOSTART_PROGRAM = 147,
    CTRL_AUTOSTOP_PROGRAM = 148,

    // Рукава тормозной магистрали
    CTRL_HOSE_BP_FWD = 149,
    CTRL_HOSE_BP_BWD = 150,

    // Рукава питательной магистрали
    CTRL_HOSE_FL_FWD = 151,
    CTRL_HOSE_FL_BWD = 152,

    // Рукава магистрали тормозных цилиндров
    CTRL_HOSE_BC_FWD = 153,
    CTRL_HOSE_BC_BWD = 154,

    // Незадействованные ранее тумблеры (управление только мышью)
    CTRL_TUMBLER_RADIO = 155,
    CTRL_TUMBLER_AUTOSAND = 156,
    CTRL_TUMBLER_P_TIFON = 157,        // кнопка
    CTRL_TUMBLER_P_WHISTLE = 158,      // кнопка
    CTRL_TUMBLER_P_CAB_HEAT = 159,
    CTRL_TUMBLER_P_SHASSIS_LIGHT = 160,
    CTRL_TUMBLER_P_ALSN_CHECK = 161,
    CTRL_TUMBLER_P_RESERVE1 = 162,
    CTRL_TUMBLER_P_RESERVE2 = 163,
};

#endif

#ifndef     HARDWARE_SIGNALS_H
#define     HARDWARE_SIGNALS_H

enum ControllerSignals
{
    KM_READY = 0,

    KM_K01 = 1,
    KM_K02 = 2,

    KM_K21 = 3,
    KM_K22 = 4,
    KM_K23 = 5,
    KM_K25 = 6,

    KM_K31 = 7,
    KM_K32 = 8,
    KM_K33 = 9,

    KM_SVISTOK = 10,
    KM_STURVAL_DOWN = 11,

    SWP1_EPT = 12,
    SWP1_MK1 = 13,
    SWP1_TP1 = 14,
    SWP1_BV = 15,
    SWP1_VK = 16,
    SWP1_TP2 = 17,

    SWP2_MK2 = 18,
    SWP2_MV = 19,
    SWP2_CAB_LIGHT = 20,
    SWP2_BF_LEFT = 21,
    SWP2_BF_RIGHT = 22,
    SWP2_SPOT_LIGHT = 23,


    EDT_OFF = 24,
    EDT_ON = 25,
    EDT_SBROS = 26,
    LOCO_RELEASE = 27,
    EDT_RELEASE = 28,
    EDT_BRAKE = 29,


    KRM_1 = 30,
    KRM_2 = 31,
    KRM_3 = 32,
    KRM_4 = 33,
    KRM_5 = 34,
    KRM_6 = 35,
    KRM_7 = 36,

    LOCO_CRANE = 37,

    KPD3_READY = 38
};

enum FeedbackSignals
{
    MAN_TM = 0,
    MAN_UR = 1,
    MAN_ZT = 2,
    MAN_GR = 3,
    MAN_TC = 4,

    SWP1_POWER_0 = 5,
    SWP1_POWER_1 = 6,
    SWP1_POWER_2 = 7,
    SWP1_POWER_3 = 8,
    SWP1_POWER_4 = 9,
    SWP1_POWER_5 = 10,
    SWP1_POWER_6 = 11,
    SWP1_POWER_7 = 12,
    SWP1_POWER_8 = 13,
    SWP1_POWER_9 = 14,
    SWP1_POWER_10 = 15,

    SWP2_POWER_0 = 16,
    SWP2_POWER_1 = 17,
    SWP2_POWER_2 = 18,
    SWP2_POWER_3 = 19,
    SWP2_POWER_4 = 20,
    SWP2_POWER_5 = 21,
    SWP2_POWER_6 = 22,
    SWP2_POWER_7 = 23,
    SWP2_POWER_8 = 24,
    SWP2_POWER_9 = 25,
    SWP2_POWER_10 = 26,

    VOLT_BAT = 27,
    VOLT_EPT = 28,
    VOLT_NETWORK = 29,
    AMPER_1_2 = 30,
    AMPER_3_4 = 31,
    AMPER_5_6 = 32,
    POS_INDICATOR = 33,

    LAMP_P = 34,
    LAMP_SP = 35,
    LAMP_S = 36,
    LAMP_ZERO = 37,

    LAMP_R = 38,
    LAMP_T = 39,
    LAMP_PEREKRISHA = 40,
    LAMP_O = 41,

    LAMP_NAR_SYNC = 42,
    LAMP_NO_BRAKES_RELEASE = 43,
    LAMP_PESOK = 44,
    LAMP_ZASHITA = 45,

    LAMP_GALYZI = 46,
    LAMP_MK = 47,
    LAMP_ACCUM2 = 48,
    LAMP_ACCUM1 = 49,

    LAMP_RAZED = 50,

    SV_LAMP_G = 51,
    SV_LAMP_R = 52,
    SV_LAMP_W = 53,
    SV_LAMP_Y = 54,
    SV_LAMP_YR = 55,

    IND_BV = 56,

    KPD3_STRELKA = 57,
    KPD3_VELOCITY = 58,
    KPD3_TARGET_DISTANCE = 59

};

#endif // HARDWARESIGNALS_H

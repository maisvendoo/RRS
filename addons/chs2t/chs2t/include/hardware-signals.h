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


    EDT_T = 24,
    EDT_D = 25,
    EDT_SBROS = 26,
    EDT_CHECK_RT = 27,
    EDT_RELEASE = 28,
    EDT_BRAKE = 29
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
    SWP2_POWER_10 = 26
};

#endif // HARDWARESIGNALS_H

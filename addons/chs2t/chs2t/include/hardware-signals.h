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


    EDT_T = 23,
    EDT_D = 24,
    EDT_SBROS = 25,
    EDT_CHECK_RT = 26,
    EDT_RELEASE = 27,
    EDT_BRAKE = 28
};

enum FeedbackSignals
{
    MAN_TM = 0,
    MAN_UR = 1,
    MAN_ZT = 2,
    MAN_GR = 3,
    MAN_TC = 4,

    POWER_0 = 5,
    POWER_1 = 6,
    POWER_2 = 7,
    POWER_3 = 8,
    POWER_4 = 9,
    POWER_5 = 10,
    POWER_6 = 11,
    POWER_7 = 12,
    POWER_8 = 13,
    POWER_9 = 14,
    POWER_10 = 15
};

#endif // HARDWARESIGNALS_H

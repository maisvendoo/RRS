#ifndef EP20_SIGNALS_H
#define EP20_SIGNALS_H

enum
{
    KONTROLLER = 0,
    RUK_KRM130 = 1,
    RUK_KVT224 = 2,
    KMB2_Real = 3,
    KMB2_Fake = 4,
    KeyCard_Fake = 5,
    KeyCard_Low = 6,
    Key = 7,


    RB = 8,
    RBP = 9,

    STRELKA_pTM = 10,
    STRELKA_pGR = 11,
    STRELKA_pTC = 12,
    STRELKA_pUR = 13,

    Reserv_break_loco = 14,
    Resevr_break_train = 15,

    Svistok_Button = 16,
    Tifon_Button = 17,

    BLOK_TM_PRESS = 100,
    BLOK_UR_PRESS = 101,
    BLOK_TC_PRESS = 102,

    BLOK_RAILWAY_COORD = 103,
    BLOK_VELOCITY = 104,
    BLOK_VELOCITY_CURRENT_LIMIT = 105,
    BLOK_VELOCITY_NEXT_LIMIT = 106,

    PANTOGRAPH_AC1 = 107,
    PANTOGRAPH_DC1 = 108,
    PANTOGRAPH_AC2 = 109,
    PANTOGRAPH_DC2 = 110,

    sigLight_Pant_fwd = 140,
    sigLight_Pant_bwd = 143,
    sigLight_GV = 146,
    sigLight_Train_heating = 149,
    sigLight_Recap_disable = 152,
    sigLight_AutoDriver = 155,
    sigLight_SpeedControl = 158,
    sigLight_VZ = 161,
    sigLight_EPT = 164,
    sigLight_GS = 167,
    sigLight_PV = 170,
    sigLight_Whell_clean = 173,
    sigLight_Saund1 = 176,
    sigLight_Brake_release = 179,
    sigLight_Test = 182,
    sigLight_Res_Purge = 185,

    LS_G4 = 186,
    LS_G3 = 187,
    LS_G2 = 188,
    LS_G1 = 189,
    LS_Y = 190,
    LS_RY = 191,
    LS_R = 192,
    LS_W = 193,

    WHEEL_1 = 194,
    WHEEL_2 = 195,
    WHEEL_3 = 196,
    WHEEL_4 = 197,
    WHEEL_5 = 198,
    WHEEL_6 = 199
};

#endif // EP20_SIGNALS_H

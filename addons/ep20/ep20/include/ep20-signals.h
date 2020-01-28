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


    STRELKA_pTM = 10,
    STRELKA_pGR = 11,
    STRELKA_pTC = 12,
    STRELKA_pUR = 13,

    BLOK_TM_PRESS = 100,
    BLOK_UR_PRESS = 101,
    BLOK_TC_PRESS = 102,

    BLOK_RAILWAY_COORD = 103,
    BLOK_VELOCITY = 104,
    BLOK_VELOCITY_CURRENT_LIMIT = 105,
    BLOK_VELOCITY_NEXT_LIMIT = 106
};

#endif // EP20_SIGNALS_H

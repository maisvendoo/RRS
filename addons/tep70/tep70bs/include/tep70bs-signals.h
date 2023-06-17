#ifndef     TEP70_SIGNALS_H
#define     TEP70_SIGNALS_H

enum
{
    KM_SHTURVAL = 0,
    KM_REVERSOR = 1,

    KRM_395 = 2,
    KVT_254_ROTATE = 3,
    KVT_254_TRANSLATE = 4,

    BRAKE_FORCE = 5,

    AZV_COMMON_CONTROL = 6,
    AZV_UPR_TEPL = 7,
    AZV_FUEL_PUMP = 8,
    AZV_PANEL_LIGHT = 9,
    AZV_CABINE_LIGHT = 10,
    AZV_EDT_ON = 11,
    AZV_EDT_POWER = 12,
    AZV_EPT_ON = 13,
    AZV_SPOTLIGHT_LOW = 14,
    AZV_SPOTLIGHT_HIGH = 15,

    TUMBLER_FIELD_WEAK1 = 16,
    TUMBLER_FIELD_WEAK2 = 17,
    TUMBLER_WATER_ZALUZI = 18,
    TUMBLER_OIL_ZALUZI = 65,

    BUTTON_DISEL_START = 19,
    BUTTON_BRAKE_RELEASE = 20,
    BUTTON_SVISTOK = 21,
    BUTTON_TIFON = 22,

    SIGLIGHT_EPT_O = 23,
    SIGLIGHT_EPT_P = 24,
    SIGLIGHT_EPT_T = 25,

    SIGLIGHT_EDT1 = 26,
    SIGLIGHT_EDT2 = 27,

    SIGLIGHT_RESET_POWER = 28,
    SIGLIGHT_OIL_TEMP = 29,
    SIGLIGHT_OIL_PRESS = 30,
    SIGLIGHT_ZHAL_ET1 = 31,
    SIGLIGHT_ZHAL_ET2 = 32,
    SIGLIGHT_ZB = 33,

    SIGLIGHT_RES_FUELPUMP = 34,
    SIGLIGHT_FIRE_L = 35,
    SIGLIGHT_FIRE_R = 36,
    SIGLIGHT_BRAKE_RESISTORS = 37,
    SIGLIGHT_KALORIPHER = 38,

    SIGLIGHT_PROP = 39,
    SIGLIGHT_PSS = 40,

    STRELKA_UR = 41,
    STRELKA_TM = 42,
    STRELKA_PM = 43,
    STRELKA_TC1 = 44,
    STRELKA_TC2 = 45,

    STRELKA_REF_SPEED = 46,
    STRELKA_REOSTATE_CURRENT = 47,
    STRELKA_GEN_CURRENT = 48,

    STRELKA_WATER_TEMP = 49,
    STRELKA_OIL_TEMP = 50,
    STRELKA_OIL_PRESS = 51,
    STRELKA_FUEL_PRESS = 64,

    STRELKA_BAT_VOLTAGE = 52,
    STRELKA_BAT_CURRENT = 53,
    STRELKA_GEN_VOLTAGE = 54,
    STRELKA_FAILS_INDICATOR = 55,

    STRELKA_SPEED = 56,


    LS_G = 57,
    LS_Y = 58,
    LS_YR = 59,
    LS_R = 60,
    LS_W = 61,

    TUMBLER_VOLTMETER = 62,
    TUMBLER_DISEL_STOP = 63,

    RUK_367 = 66,
    COMB_KRAN = 67,
    KRAN_395_RUK = 68,
    KRAN_254_POD = 69,
    KRAN_254_RUK = 70,

    KLUCH_EPK = 71,
    RB1 = 72,

    VAL_PRSKOR1 = 73,
    VAL_PRSKOR2 = 74,

    KLUB_ON = 90,
    KLUB_ALARM = 91,



    MSUT_NOMER_KADR_DISP = 171, // номер кадра дисплея
    MSUT_TIMER_OSTANOV = 172,
    MSUT_TIMER_PROKRUTKA = 173,
    MSUT_TIMER_PROKACHKA = 174,
    MSUT_U_CHAIN = 175,     // Напряжение борт. сети
    MSUT_P_FUEL = 176,      // Давление топлива
    MSUT_I_AB = 177,        // Ток заряда АБ
    MSUT_T_OIL = 178,       // Температура масла
    MSUT_T_WHATER = 179,    // Температура воды
    MSUT_P_OIL = 180,       // Давление масла
    MSUT_POWER_OTOPLENIE = 181, // мощность на отопление
    MSUT_POWER = 182,           // мощность
    MSUT_VU2_I = 183,
    MSUT_VU2_U = 184,
    MSUT_VU1_U = 185,
    MSUT_VU1_I = 186,
    MSUT_VU1_I_TED = 187,
    MSUT_MODE = 188,            // РЕЖИМ
    MSUT_POSITION = 189,        // ПОЗИЦИЯ
    MSUT_REVERSOR = 190,        // РЕВЕРСОР
    MSUT_ET_T = 191,            // ЭТ/Тяга
    MSUT_ACCELLERATION = 192,   // Ускорение
    MSUT_SPEED = 193,           // Скорость

    WHEEL_1 = 194,
    WHEEL_2 = 195,
    WHEEL_3 = 196,
    WHEEL_4 = 197,
    WHEEL_5 = 198,
    WHEEL_6 = 199
};

#endif // TEP70_SIGNALS_H

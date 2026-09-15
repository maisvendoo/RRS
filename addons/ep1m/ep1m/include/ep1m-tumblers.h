#ifndef     EP1M_TUMBLERS_H
#define     EP1M_TUMBLERS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    SWITCHERS_COUNT = 2,
    TUMBLERS_COUNT = 32
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    SWITCHER_PANEL_BRIGHTNESS = 0,
    SWITCHER_DEVICES_BRIGHTNESS = 1
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    // Панель на столешнице справа
    TUMBLER_AUTOSAND = 1,
    TUMBLER_SAND_ON_EMERGENCY = 2,
    TUMBLER_MPK = 3,
    BUTTON_RETURN_AUX_PROTECTION = 4,
    TUMBLER_AUTO_MODE = 5,

    // Панель на средней тумбе
    TUMBLER_BUFFERCOLOR_L = 6,
    TUMBLER_BUFFERLIGHT_L = 7,
    TUMBLER_BUFFERLIGHT_R = 8,
    TUMBLER_BUFFERCOLOR_R = 9,

    // Панель слева
    TUMBLER_SHASSIS_LIGHT = 10,
    TUMBLER_SPOTLIGHT_LOW = 11,
    TUMBLER_SPOTLIGHT_HIGH = 12,
    TUMBLER_CAB_LIGHT_LOW = 13,
    TUMBLER_CAB_LIGHT_HIGH = 14,
    TUMBLER_CAB_LIGHT_GREEN = 15,
    TUMBLER_DEVICES_LIGHT = 16,
    TUMBLER_BATTERY_OR_EPB_VOLTAGE = 17,

    // Панель на столешнице слева
    BUTTON_RELEASE_BRAKES = 18,
    BUTTON_COMPRESSOR = 19,
    TUMBLER_SIGNAL_PANEL_BS_002 = 20,
    TUMBLER_PCHF = 21,
    TUMBLER_USAVP = 22,

    BUTTON_RB = 23,
    BUTTON_RBS = 24,
    BUTTON_RBP = 25,

    // Пульт помощника
    BUTTON_EMERGENCY_BRAKE = 26,
    BUTTON_MAIN_SWITCH_OFF = 27,
    BUTTON_P_TYPHON = 28,
    BUTTON_P_WHISTLE = 29,
};

#endif // EP1M_TUMBLERS_H

#ifndef     EP1M_CONTROLS_H
#define     EP1M_CONTROLS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    // Ключ блокировки панели тумблеров: 0 - вынут, 1 - вставлен, 2 - повёрнут
    CTRL_EP1M_PANEL_KEY = 100,
    // Тумблеры панели (EP1MTumblersPanel)
    CTRL_EP1M_TUMBLER_MSUD = 101,
    CTRL_EP1M_TUMBLER_LOCK_VVK = 102,
    CTRL_EP1M_TUMBLER_PANT1 = 103,
    CTRL_EP1M_TUMBLER_PANT2 = 104,
    CTRL_EP1M_TUMBLER_RETURN_PROTECTION = 105,
    CTRL_EP1M_TUMBLER_MAIN_SWITCH = 106,
    CTRL_EP1M_TUMBLER_AUX_MACHINES = 107,
    CTRL_EP1M_TUMBLER_COMPRESSOR = 108,
    CTRL_EP1M_TUMBLER_FAN1 = 109,
    CTRL_EP1M_TUMBLER_FAN2 = 110,
    CTRL_EP1M_TUMBLER_FAN3 = 111,
    CTRL_EP1M_TUMBLER_EPT = 112,
    // Тумблеры пульта (tumblers[])
    CTRL_EP1M_TUMBLER_MPK = 113,
    CTRL_EP1M_TUMBLER_AUTO_MODE = 114,
    CTRL_EP1M_TUMBLER_BUFFERLIGHT_L = 115,
    CTRL_EP1M_TUMBLER_BUFFERLIGHT_R = 116,
    CTRL_EP1M_TUMBLER_BUFFERCOLOR_L = 117,
    CTRL_EP1M_TUMBLER_BUFFERCOLOR_R = 118,
    CTRL_EP1M_TUMBLER_SPOTLIGHT_LOW = 119,
    CTRL_EP1M_TUMBLER_SPOTLIGHT_HIGH = 120,
    CTRL_EP1M_TUMBLER_CAB_LIGHT_LOW = 121,
    CTRL_EP1M_TUMBLER_CAB_LIGHT_HIGH = 122,
    CTRL_EP1M_TUMBLER_CAB_LIGHT_GREEN = 123,
    CTRL_EP1M_TUMBLER_DEVICES_LIGHT = 124,
    CTRL_EP1M_TUMBLER_BATTERY_OR_EPB = 125,
    CTRL_EP1M_TUMBLER_SIGNAL_PANEL = 126,
    CTRL_EP1M_TUMBLER_PCHF = 127,
    // Поездной кран 395: позиция I..VI (0..6)
    CTRL_EP1M_CRANE_395 = 140,
    // Кран вспомогательного тормоза (КЖТ): целевое давление ТЦ 0..1
    CTRL_EP1M_CRANE_254 = 141,
    // КМ-35: главная рукоятка, уровень 0..1 (тяга)
    CTRL_EP1M_KM_LEVEL = 142,
    // КМ-35: рукоятка режима: -1 маневровый, 0 нулевой, 1 поездной
    CTRL_EP1M_KM_MODE = 143,
    // КМ-35: реверс: -1 назад, 0 ноль, 1 вперёд
    CTRL_EP1M_KM_REVERS = 144,
    // УБТ-367: поворот ключа
    CTRL_EP1M_LOCK_367 = 145,
    // Комбинированный кран: -1 двойная тяга / 0 поездное / +1 экстренное
    CTRL_EP1M_COMBINE_CRANE = 146,
    // Ключ ЭПК: 0 - вынут, 1 - вставлен, 2 - повёрнут
    CTRL_EP1M_EPK_KEY = 147,
    // КМ-35: вставка/извлечение реверсивной рукоятки
    CTRL_EP1M_KM_REVERS_INSERT = 148,
    // Моментальные кнопки (удержание мыши = удержание кнопки)
    CTRL_EP1M_BUTTON_TYPHON = 150,
    CTRL_EP1M_BUTTON_WHISTLE = 151,
    CTRL_EP1M_BUTTON_SAND = 152,
    CTRL_EP1M_BUTTON_RB = 153,
    CTRL_EP1M_BUTTON_RBS = 154
};

#endif

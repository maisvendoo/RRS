//------------------------------------------------------------------------------
//
//      Keyboard keys for controls from viewer client
//      (c) maisvendoo, 17/02/2019
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Keyboard keys for controls from viewer client
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 17/02/2019
 */

#ifndef     KEY_SYMBOLS_H
#define     KEY_SYMBOLS_H

#include <cstdint>
#include <set>

/*!
 * \enum
 * \brief Key codes
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum KeySymbol
{
    KEY_Undefined = 0x0,

    KEY_Space           = 0x20, ///< Пробел

    KEY_0               = '0',
    KEY_1               = '1',
    KEY_2               = '2',
    KEY_3               = '3',
    KEY_4               = '4',
    KEY_5               = '5',
    KEY_6               = '6',
    KEY_7               = '7',
    KEY_8               = '8',
    KEY_9               = '9',

    KEY_A               = 'a', ///<| ф |
    KEY_B               = 'b', ///<| и |
    KEY_C               = 'c', ///<| с |
    KEY_D               = 'd', ///<| в |
    KEY_E               = 'e', ///<| у |
    KEY_F               = 'f', ///<| а |
    KEY_G               = 'g', ///<| п |
    KEY_H               = 'h', ///<| р |
    KEY_I               = 'i', ///<| ш |
    KEY_J               = 'j', ///<| о |
    KEY_K               = 'k', ///<| л |
    KEY_L               = 'l', ///<| д |
    KEY_M               = 'm', ///<| ь |
    KEY_N               = 'n', ///<| т |
    KEY_O               = 'o', ///<| щ |
    KEY_P               = 'p', ///<| з |
    KEY_Q               = 'q', ///<| й |
    KEY_R               = 'r', ///<| к |
    KEY_S               = 's', ///<| ы |
    KEY_T               = 't', ///<| е |
    KEY_U               = 'u', ///<| г |
    KEY_V               = 'v', ///<| м |
    KEY_W               = 'w', ///<| ц |
    KEY_X               = 'x', ///<| ч |
    KEY_Y               = 'y', ///<| н |
    KEY_Z               = 'z', ///<| я |

    KEY_Quote           = 0x27, ///<| ' | э |
    KEY_Comma           = 0x2C, ///<| , | б |
    KEY_Minus           = 0x2D, ///<| - |
    KEY_Period          = 0x2E, ///<| . | ю |
    KEY_Slash           = 0x2F, ///<| / | . |
    KEY_Semicolon       = 0x3B, ///<| ; | ж |
    KEY_Equals          = 0x3D, ///<| = |
    KEY_Leftbracket     = 0x5B, ///<| [ | х |
    KEY_Backslash       = 0x5C, ///<| \ |
    KEY_Rightbracket    = 0x5D, ///<| ] | ъ |
    KEY_Tilde           = 0x7E, ///<| ~ | ё |

    KEY_BackSpace       = 0xFF08, ///< BackSpace
    KEY_Tab             = 0xFF09, ///< Tab
    KEY_Return          = 0xFF0D, ///< Return, Enter
    KEY_Scroll_Lock     = 0xFF14, ///< Scroll Lock
    KEY_Escape          = 0xFF1B, ///< Escape
    KEY_Print           = 0xFF61, ///< PrintScreen

    KEY_Home            = 0xFF50, ///< Home         / also Numpad 7 if Num Lock off
    KEY_Left            = 0xFF51, ///< Left arrow   / also Numpad 4 if Num Lock off
    KEY_Up              = 0xFF52, ///< Up arrow     / also Numpad 8 if Num Lock off
    KEY_Right           = 0xFF53, ///< Right arrow  / also Numpad 6 if Num Lock off
    KEY_Down            = 0xFF54, ///< Down arrow   / also Numpad 2 if Num Lock off
    KEY_Page_Up         = 0xFF55, ///< Page Up      / also Numpad 9 if Num Lock off
    KEY_Page_Down       = 0xFF56, ///< Page Down    / also Numpad 3 if Num Lock off
    KEY_End             = 0xFF57, ///< End          / also Numpad 1 if Num Lock off
    KEY_Insert          = 0xFF63, ///< Insert       / also Numpad 0 if Num Lock off
    KEY_Delete          = 0xFFFF, ///< Delete       / also Numpad Decimal if Num Lock off

    KEY_KP_Enter        = 0xFF8D, ///< (numpad) Enter
    KEY_KP_Multiply     = 0xFFAA, ///< (numpad) | * |
    KEY_KP_Add          = 0xFFAB, ///< (numpad) | + |
    KEY_KP_Subtract     = 0xFFAD, ///< (numpad) | - |
    KEY_KP_Decimal      = 0xFFAE, ///< (numpad) | . |
    KEY_KP_Divide       = 0xFFAF, ///< (numpad) | / |
    KEY_KP_0            = 0xFFB0,
    KEY_KP_1            = 0xFFB1,
    KEY_KP_2            = 0xFFB2,
    KEY_KP_3            = 0xFFB3,
    KEY_KP_4            = 0xFFB4,
    KEY_KP_5            = 0xFFB5,
    KEY_KP_6            = 0xFFB6,
    KEY_KP_7            = 0xFFB7,
    KEY_KP_8            = 0xFFB8,
    KEY_KP_9            = 0xFFB9,

    KEY_F1              = 0xFFBE,
    KEY_F2              = 0xFFBF,
    KEY_F3              = 0xFFC0,
    KEY_F4              = 0xFFC1,
    KEY_F5              = 0xFFC2,
    KEY_F6              = 0xFFC3,
    KEY_F7              = 0xFFC4,
    KEY_F8              = 0xFFC5,
    KEY_F9              = 0xFFC6,
    KEY_F10             = 0xFFC7,
    KEY_F11             = 0xFFC8,
    KEY_F12             = 0xFFC9,

    KEY_Shift_L         = 0xFFE1, ///< Left shift
    KEY_Shift_R         = 0xFFE2, ///< Right shift
    KEY_Control_L       = 0xFFE3, ///< Left control
    KEY_Control_R       = 0xFFE4, ///< Right control
    KEY_Alt_L           = 0xFFE9, ///< Left alt
    KEY_Alt_R           = 0xFFEA, ///< Right alt
    KEY_Super_L         = 0xFFEB, ///< Left super, Left Win
    KEY_Super_R         = 0xFFEC, ///< Right super, Right Win
};

const std::set<uint16_t> KeySymbolsRRS =
    { KEY_Space
    , KEY_Quote
    , KEY_Comma
    , KEY_Minus
    , KEY_Period
    , KEY_Slash
    , KEY_0
    , KEY_1
    , KEY_2
    , KEY_3
    , KEY_4
    , KEY_5
    , KEY_6
    , KEY_7
    , KEY_8
    , KEY_9
    , KEY_Semicolon
    , KEY_Equals
    , KEY_Leftbracket
    , KEY_Backslash
    , KEY_Rightbracket
    , KEY_A
    , KEY_B
    , KEY_C
    , KEY_D
    , KEY_E
    , KEY_F
    , KEY_G
    , KEY_H
    , KEY_I
    , KEY_J
    , KEY_K
    , KEY_L
    , KEY_M
    , KEY_N
    , KEY_O
    , KEY_P
    , KEY_Q
    , KEY_R
    , KEY_S
    , KEY_T
    , KEY_U
    , KEY_V
    , KEY_W
    , KEY_X
    , KEY_Y
    , KEY_Z
    , KEY_Tilde
    , KEY_BackSpace
//    , KEY_Tab
//    , KEY_Return
//    , KEY_Scroll_Lock
//    , KEY_Escape
//    , KEY_Home
//    , KEY_Left
//    , KEY_Up
//    , KEY_Right
//    , KEY_Down
//    , KEY_Page_Up
//    , KEY_Page_Down
//    , KEY_End
//    , KEY_Print
    , KEY_Insert
//    , KEY_KP_Enter
    , KEY_KP_Multiply
    , KEY_KP_Add
    , KEY_KP_Subtract
    , KEY_KP_Decimal
    , KEY_KP_Divide
    , KEY_KP_0
    , KEY_KP_1
    , KEY_KP_2
    , KEY_KP_3
    , KEY_KP_4
    , KEY_KP_5
    , KEY_KP_6
    , KEY_KP_7
    , KEY_KP_8
    , KEY_KP_9
    , KEY_F1
    , KEY_F2
    , KEY_F3
    , KEY_F4
    , KEY_F5
    , KEY_F6
    , KEY_F7
    , KEY_F8
    , KEY_F9
    , KEY_F10
    , KEY_F11
    , KEY_F12
    , KEY_Shift_L
    , KEY_Shift_R
    , KEY_Control_L
    , KEY_Control_R
    , KEY_Alt_L
    , KEY_Alt_R
//    , KEY_Super_L
//    , KEY_Super_R
    , KEY_Delete
};

#endif // KEY_SYMBOLS_H

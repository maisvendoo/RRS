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

/*!
 * \enum
 * \brief Key codes
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum KeySymbol
{
    KEY_Space           = 0x20,

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
    KEY_A               = 'a',
    KEY_B               = 'b',
    KEY_C               = 'c',
    KEY_D               = 'd',
    KEY_E               = 'e',
    KEY_F               = 'f',
    KEY_G               = 'g',
    KEY_H               = 'h',
    KEY_I               = 'i',
    KEY_J               = 'j',
    KEY_K               = 'k',
    KEY_L               = 'l',
    KEY_M               = 'm',
    KEY_N               = 'n',
    KEY_O               = 'o',
    KEY_P               = 'p',
    KEY_Q               = 'q',
    KEY_R               = 'r',
    KEY_S               = 's',
    KEY_T               = 't',
    KEY_U               = 'u',
    KEY_V               = 'v',
    KEY_W               = 'w',
    KEY_X               = 'x',
    KEY_Y               = 'y',
    KEY_Z               = 'z',

    KEY_Exclaim         = 0x21,
    KEY_Quotedbl        = 0x22,
    KEY_Hash            = 0x23,
    KEY_Dollar          = 0x24,
    KEY_Ampersand       = 0x26,
    KEY_Quote           = 0x27,
    KEY_Leftparen       = 0x28,
    KEY_Rightparen      = 0x29,
    KEY_Asterisk        = 0x2A,
    KEY_Plus            = 0x2B,
    KEY_Comma           = 0x2C,
    KEY_Minus           = 0x2D,
    KEY_Period          = 0x2E,
    KEY_Slash           = 0x2F,
    KEY_Colon           = 0x3A,
    KEY_Semicolon       = 0x3B,
    KEY_Less            = 0x3C,
    KEY_Equals          = 0x3D,
    KEY_Greater         = 0x3E,
    KEY_Question        = 0x3F,
    KEY_At              = 0x40,
    KEY_Leftbracket     = 0x5B,
    KEY_Backslash       = 0x5C,
    KEY_Rightbracket    = 0x5D,
    KEY_Caret           = 0x5E,
    KEY_Underscore      = 0x5F,
    KEY_Backquote       = 0x60,

    KEY_BackSpace       = 0xFF08,        /* back space, back char */
    KEY_Tab             = 0xFF09,
    KEY_Linefeed        = 0xFF0A,        /* Linefeed, LF */
    KEY_Clear           = 0xFF0B,
    KEY_Return          = 0xFF0D,        /* Return, enter */
    KEY_Pause           = 0xFF13,        /* Pause, hold */
    KEY_Scroll_Lock     = 0xFF14,
    KEY_Sys_Req         = 0xFF15,
    KEY_Escape          = 0xFF1B,
    KEY_Delete          = 0xFFFF,        /* Delete, rubout */


    /* Cursor control & motion */

    KEY_Home            = 0xFF50,
    KEY_Left            = 0xFF51,        /* Move left, left arrow */
    KEY_Up              = 0xFF52,        /* Move up, up arrow */
    KEY_Right           = 0xFF53,        /* Move right, right arrow */
    KEY_Down            = 0xFF54,        /* Move down, down arrow */
    KEY_Prior           = 0xFF55,        /* Prior, previous */
    KEY_Page_Up         = 0xFF55,
    KEY_Next            = 0xFF56,        /* Next */
    KEY_Page_Down       = 0xFF56,
    KEY_End             = 0xFF57,        /* EOL */
    KEY_Begin           = 0xFF58,        /* BOL */


    /* Misc Functions */

    KEY_Select          = 0xFF60,        /* Select, mark */
    KEY_Print           = 0xFF61,
    KEY_Execute         = 0xFF62,        /* Execute, run, do */
    KEY_Insert          = 0xFF63,        /* Insert, insert here */
    KEY_Undo            = 0xFF65,        /* Undo, oops */
    KEY_Redo            = 0xFF66,        /* redo, again */
    KEY_Menu            = 0xFF67,        /* On Windows, this is VK_APPS, the context-menu key */
    KEY_Find            = 0xFF68,        /* Find, search */
    KEY_Cancel          = 0xFF69,        /* Cancel, stop, abort, exit */
    KEY_Help            = 0xFF6A,        /* Help */
    KEY_Break           = 0xFF6B,
    KEY_Mode_switch     = 0xFF7E,        /* Character set switch */
    KEY_Script_switch   = 0xFF7E,        /* Alias for mode_switch */
    KEY_Num_Lock        = 0xFF7F,

    /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

    KEY_KP_Space        = 0xFF80,        /* space */
    KEY_KP_Tab          = 0xFF89,
    KEY_KP_Enter        = 0xFF8D,        /* enter */
    KEY_KP_F1           = 0xFF91,        /* PF1, KP_A, ... */
    KEY_KP_F2           = 0xFF92,
    KEY_KP_F3           = 0xFF93,
    KEY_KP_F4           = 0xFF94,
    KEY_KP_Home         = 0xFF95,
    KEY_KP_Left         = 0xFF96,
    KEY_KP_Up           = 0xFF97,
    KEY_KP_Right        = 0xFF98,
    KEY_KP_Down         = 0xFF99,
    KEY_KP_Prior        = 0xFF9A,
    KEY_KP_Page_Up      = 0xFF9A,
    KEY_KP_Next         = 0xFF9B,
    KEY_KP_Page_Down    = 0xFF9B,
    KEY_KP_End          = 0xFF9C,
    KEY_KP_Begin        = 0xFF9D,
    KEY_KP_Insert       = 0xFF9E,
    KEY_KP_Delete       = 0xFF9F,
    KEY_KP_Equal        = 0xFFBD,        /* equals */
    KEY_KP_Multiply     = 0xFFAA,
    KEY_KP_Add          = 0xFFAB,
    KEY_KP_Separator    = 0xFFAC,       /* separator, often comma */
    KEY_KP_Subtract     = 0xFFAD,
    KEY_KP_Decimal      = 0xFFAE,
    KEY_KP_Divide       = 0xFFAF,

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

    /*
     * Auxiliary Functions; note the duplicate definitions for left and right
     * function keys;  Sun keyboards and a few other manufactures have such
     * function key groups on the left and/or right sides of the keyboard.
     * We've not found a keyboard with more than 35 function keys total.
     */

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
    KEY_F13             = 0xFFCA,
    KEY_F14             = 0xFFCB,
    KEY_F15             = 0xFFCC,
    KEY_F16             = 0xFFCD,
    KEY_F17             = 0xFFCE,
    KEY_F18             = 0xFFCF,
    KEY_F19             = 0xFFD0,
    KEY_F20             = 0xFFD1,
    KEY_F21             = 0xFFD2,
    KEY_F22             = 0xFFD3,
    KEY_F23             = 0xFFD4,
    KEY_F24             = 0xFFD5,
    KEY_F25             = 0xFFD6,
    KEY_F26             = 0xFFD7,
    KEY_F27             = 0xFFD8,
    KEY_F28             = 0xFFD9,
    KEY_F29             = 0xFFDA,
    KEY_F30             = 0xFFDB,
    KEY_F31             = 0xFFDC,
    KEY_F32             = 0xFFDD,
    KEY_F33             = 0xFFDE,
    KEY_F34             = 0xFFDF,
    KEY_F35             = 0xFFE0,

    /* Modifiers */

    KEY_Shift_L         = 0xFFE1,        /* Left shift */
    KEY_Shift_R         = 0xFFE2,        /* Right shift */
    KEY_Control_L       = 0xFFE3,        /* Left control */
    KEY_Control_R       = 0xFFE4,        /* Right control */
    KEY_Caps_Lock       = 0xFFE5,        /* Caps lock */
    KEY_Shift_Lock      = 0xFFE6,        /* Shift lock */

    KEY_Meta_L          = 0xFFE7,        /* Left meta */
    KEY_Meta_R          = 0xFFE8,        /* Right meta */
    KEY_Alt_L           = 0xFFE9,        /* Left alt */
    KEY_Alt_R           = 0xFFEA,        /* Right alt */
    KEY_Super_L         = 0xFFEB,        /* Left super */
    KEY_Super_R         = 0xFFEC,        /* Right super */
    KEY_Hyper_L         = 0xFFED,        /* Left hyper */
    KEY_Hyper_R         = 0xFFEE         /* Right hyper */
};

#endif // KEY_SYMBOLS_H

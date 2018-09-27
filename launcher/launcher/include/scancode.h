//------------------------------------------------------------------------------
//
//      Scancodes for special keys in different platforms
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Scancodes for special keys in different platforms
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     SCANCODE_H
#define     SCANCODE_H

#include    <QtGlobal>

//------------------------------------------------------------------------------
//      Platform depended scancodes
//------------------------------------------------------------------------------
#if defined(Q_OS_WIN)
    enum
    {
        KEY_L_SHIFT = 42,
        KEY_R_SHIFT = 54,
        KEY_L_CTRL = 29,
        KEY_R_CTRL = 285,
        KEY_L_ALT = 56,
        KEY_R_ALT = 312,
        KEY_UP = 328,
        KEY_DOWN = 336,
        KEY_LEFT = 331,
        KEY_RIGHT = 333
    };
#else

#endif

#endif // SCANCODE_H

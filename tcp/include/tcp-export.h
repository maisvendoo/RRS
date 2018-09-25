//------------------------------------------------------------------------------
//
//      Macro for crossplatform classes export from library
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Macro for crossplatform classes export from library
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     TCP_EXPORT_H
#define     TCP_EXPORT_H

#include    <QtGlobal>

#if defined(TCP_LIB)
    #define TCP_EXPORT  Q_DECL_EXPORT
#else
    #define TCP_EXPORT  Q_DECL_IMPORT
#endif

#endif // TCP_EXPORT_H

//------------------------------------------------------------------------------
//
//      Strings to numbers conversion
//      (с) maisvendoo 17/09/2016
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief      Strings to numbers conversion
 *  \copyright  maisvendoo
 *  \author     Dmitry Pritykin
 *  \date       17/09/2016
 */

#ifndef     CONVERT_H
#define     CONVERT_H

#include    <QString>
#include    <QtGlobal>

/// QString to double
extern "C" Q_DECL_EXPORT bool TextToDouble(const QString& text, double& value);

/// QString to integer
extern "C" Q_DECL_EXPORT bool TextToInt(const QString& text, int& value);

/// QString to float
extern "C" Q_DECL_EXPORT bool TextToFloat(const QString& text, float& value);

/// Erase all spaces from QString
QString EraseSpaces(const QString& str);

#endif // CONVERT_H

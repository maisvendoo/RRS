//------------------------------------------------------------------------------
//
//      Strings to numbers conversion
//      (с) maisvendoo 17/09/2016
//      Devleloper: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Strings to numbers conversion
 *  \copyright maisvendoo
 *  \author Dmitry Pritykin
 *  \date  17/09/2016
 */

#include "convert.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool TextToDouble(QString text, double& value)
{
    bool validate = false; // Check data flag

    // Try data conversion
    value = text.toDouble(&validate);

    // Check validate flag
    return validate;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool TextToInt(QString text, int& value)
{
    bool validate = false;
    value = text.toInt(&validate);
    return validate;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool TextToFloat(QString text, float& value)
{
    bool validate = false;
    value = text.toFloat(&validate);
    return validate;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QString EraseSpaces(QString str)
{
    QString result = "";

    const auto length = str.length();
    for (decltype(length) i = 0; i < length; ++i)
    {
        if (str.at(i) != QChar(' '))
        {
            result += str.at(i);
        }
    }

    return result;
}

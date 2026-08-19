//------------------------------------------------------------------------------
//
//      Strings to numbers conversion
//      (с) maisvendoo 17/09/2016
//      Devleloper: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief     Strings to numbers conversion
 *  \copyright maisvendoo
 *  \author    Dmitry Pritykin
 *  \date      17/09/2016
 */

#include    "convert.h"

//------------------------------------------------------------------------------
// QString to double
//------------------------------------------------------------------------------
bool TextToDouble(const QString& text, double& value)
{
    bool validate{false}; // Check data flag

    // Try data conversion
    value = text.toDouble(&validate);

    // Check validate flag
    return validate;
}

//------------------------------------------------------------------------------
// QString to integer
//------------------------------------------------------------------------------
bool TextToInt(const QString& text, int& value)
{
    bool validate{false};
    value = text.toInt(&validate);
    return validate;
}

//------------------------------------------------------------------------------
// QString to float
//------------------------------------------------------------------------------
bool TextToFloat(const QString& text, float& value)
{
    bool validate{false};
    value = text.toFloat(&validate);
    return validate;
}

//------------------------------------------------------------------------------
// Erase all spaces from QString
//------------------------------------------------------------------------------
QString EraseSpaces(const QString& str)
{
    QString result{};

    const qsizetype length{str.length()};
    for (qsizetype i{0}; i < length; ++i)
    {
        if (str.at(i) != QChar{' '})
        {
            result += str.at(i);
        }
    }

    return result;
}

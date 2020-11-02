//------------------------------------------------------------------------------
//
//		Strings to numbers conversion
//		(с) maisvendoo 17/09/2016
//		Devleloper: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Strings to numbers conversion
 *  \copyright maisvendoo
 *  \author Dmitry Pritykin
 *  \date  17/09/2016
 */

#include	"convert.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool TextToDouble(QString text, double &value)
{
    bool validate = false;	// Check data flag

    // Try data conversion
	value = text.toDouble(&validate);
	
    // Check validate flag
	if (!validate)
    {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool TextToInt(QString text, int &value)
{
	bool validate = false;
	value = text.toInt(&validate);
	
	if (!validate)
    {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QString EraseSpaces(QString str)
{
    QString result = "";

    for (int i = 0; i < str.length(); i++)
    {
        if (str.at(i) != QChar(' '))
            result += str.at(i);
    }

    return result;
}

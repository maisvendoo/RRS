//------------------------------------------------------------------------------
//
//		XML config reader library
//		(с) maisvendoo 17/09/2016
//		Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief XML config reader library
 *  \copyright  maisvendoo
 *  \author Dmitry Pritykin
 *  \date 17/09/2016
 */

#include "CfgReader.h"
#include "convert.h"
#include <QTextStream>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CfgReader::CfgReader()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CfgReader::~CfgReader()
{
	
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::load(QString path)
{
    // Try open file
	file_name = path;
	file = new QFile(file_name);

	if (!file->open(QFile::ReadOnly | QFile::Text))
    {
		return false;
	}

    // Read content of file
	domDoc.setContent(file);

    // Close file
	file->close();

    // Get root element
	firstElement = domDoc.documentElement();

    // Check name of root element
	if (firstElement.tagName() != "Config")
    {
		return false;
	}	
	
	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getString(QString section,
                          QString field,
                          QString &value)
{
	QDomNode secNode = getFirstSection(section);

	if (secNode.isNull())
    {
		return false;
    }

	QDomNode fieldNode = getField(secNode, field);

	if (fieldNode.isNull())
    {
		return false;
	}

	value = fieldNode.toElement().text();

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getDouble(QString section, QString field, double &value)
{
	QString tmp;
    if (!getString(section, field, tmp))
	{
		return false;
	}

    if (!TextToDouble(tmp, value))
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getInt(QString section, QString field, int &value)
{
	QString tmp;
    if (!getString(section, field, tmp))
	{
		return false;
	}

    if (!TextToInt(tmp, value))
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QDomNode CfgReader::getFirstSection(QString section)
{
	QDomNode node = firstElement.firstChild();

	while ( (node.nodeName() != section) && (!node.isNull()) )
	{
		node = node.nextSibling();
	}

	curNode = node;

	return node;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QDomNode CfgReader::getNextSection()
{
	QString section = curNode.nodeName();
	QDomNode node = curNode.nextSibling();

	while ((node.nodeName() != section) && (!node.isNull()))
	{
		node = node.nextSibling();
	}

	curNode = node;

	return node;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QDomNode CfgReader::getField(QDomNode secNode, QString field)
{
	QDomNode node = secNode.firstChild();

	while ((node.nodeName() != field) && (!node.isNull()))
	{
		node = node.nextSibling();
	}

	return node;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getString(QDomNode secNode, QString field, QString &value)
{
	QDomNode node = getField(secNode, field);

	if (node.isNull())
    {
		return false;
	}

	value = node.toElement().text();

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getDouble(QDomNode secNode, QString field, double &value)
{
	QString tmp;

    if (!getString(secNode, field, tmp))
	{
		return false;
	}

    if (!TextToDouble(tmp, value))
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getInt(QDomNode secNode, QString field, int &value)
{
	QString tmp;

    if (!getString(secNode, field, tmp))
	{
		return false;
	}

    if (!TextToInt(tmp, value))
	{
		return false;
	}

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getBool(QString section, QString field, bool &value)
{
    QString tmp;
    if (!getString(section, field, tmp))
    {
        return false;
    }

    tmp = EraseSpaces(tmp);

    if ( (tmp == "True") || (tmp == "true") || (tmp == "1") )
        value = true;
    else
    {
        if ( (tmp == "False") || (tmp == "false") || (tmp == "0") )
            value = false;
        else
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CfgReader::getBool(QDomNode secNode, QString field, bool &value)
{
    QString tmp;

    if (!getString(secNode, field, tmp))
    {
        return false;
    }

    tmp = EraseSpaces(tmp);

    if ( (tmp == "True") || (tmp == "true") || (tmp == "1") )
        value = true;
    else
    {
        if ( (tmp == "False") || (tmp == "false") || (tmp == "0") )
            value = false;
        else
            return false;
    }

    return true;
}

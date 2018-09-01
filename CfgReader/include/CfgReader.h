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

#ifndef CFGREADER_H
#define CFGREADER_H

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QString>
#include <QFile>

/*!
 *  \class CfgReader
 *  \brief Work with XML config file
 */
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CfgReader
{
public:

     CfgReader();
    ~CfgReader();

    /// Loading of XML file
    bool load(QString path);

    /// Find first section by name
	QDomNode getFirstSection(QString section);
    /// Find next section
	QDomNode getNextSection();

    /// Get field in section
	QDomNode getField(QDomNode secNode, QString field);

    /// Get string field
    bool getString(QString section, QString field, QString &value);
    /// Get string field
    bool getString(QDomNode secNode, QString field, QString &value);
    /// Get double field
    bool getDouble(QString section, QString filed, double &value);
    /// Get double field
    bool getDouble(QDomNode secNode, QString field, double &value);
    /// Get integer field
    bool getInt(QString section, QString filed, int &value);
    /// Get integer field
    bool getInt(QDomNode secNode, QString field, int &value);
    /// Get boolean filed
    bool getBool(QString section, QString field, bool &value);
    /// Get boolean field
    bool getBool(QDomNode secNode, QString field, bool &value);

private:

    /// File object
    QFile *file;
    /// File name
	QString file_name;
    /// Document object
	QDomDocument domDoc;
    /// First element in document
	QDomElement firstElement;
    /// Current document node for parsed file
	QDomNode curNode;	
};

#endif // CFGREADER_H


//------------------------------------------------------------------------------
//
//		Класс для записи и редактирования конфигурационных XML-файлов
//		(с) РГУПС, ВЖД 23/05/2018
//		Разработал: Даглдиян Б.Д.
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс для записи и редактирования конфигурационных XML-файлов
 *  \copyright  РГУПС, ВЖД
 *  \author Даглдиян Б.Д.
 *  \date 23/05/2018
 */

#include    "CfgEditor.h"

#include    <QPair>
#include    <QVariant>



//-----------------------------------------------------------------------------
//  Конструктор
//-----------------------------------------------------------------------------
CfgEditor::CfgEditor()
{

}


//-----------------------------------------------------------------------------
//  Открыть файл для записи
//-----------------------------------------------------------------------------
void CfgEditor::openFileForWrite(QString fileName)
{
    file_.setFileName(fileName);
    file_.open(QIODevice::WriteOnly);

    xmlWriter_.setDevice(&file_);
    xmlWriter_.setAutoFormatting(true);
    xmlWriter_.writeStartDocument();
    xmlWriter_.writeStartElement("Config");
}

//-----------------------------------------------------------------------------
//  Записать в файл список тегов
//-----------------------------------------------------------------------------
void CfgEditor::writeFile(FieldsDataList fields_data)
{
    for (int i = 0, n = fields_data.size(); i < n; ++i)
    {
        xmlWriter_.writeStartElement(fields_data[i].first);
        xmlWriter_.writeCharacters(fields_data[i].second.toString());
        xmlWriter_.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
//  Записать в файл в ноду <sectionName> список тегов
//-----------------------------------------------------------------------------
void CfgEditor::writeFile(QString sectionName, FieldsDataList fields_data)
{
    xmlWriter_.writeStartElement(sectionName);

    writeFile(fields_data);

    xmlWriter_.writeEndElement();
}

//-----------------------------------------------------------------------------
//  Закрыть файл после записи
//-----------------------------------------------------------------------------
void CfgEditor::closeFileAfterWrite()
{
    xmlWriter_.writeEndElement();
    xmlWriter_.writeEndDocument();
    file_.close();
}

//-----------------------------------------------------------------------------
//  Формат отступов
//-----------------------------------------------------------------------------
void CfgEditor::setIndentationFormat(int numSpacesOrTabs)
{
    xmlWriter_.setAutoFormattingIndent(numSpacesOrTabs);
}

//-----------------------------------------------------------------------------
//  Редактировать файл
//-----------------------------------------------------------------------------
void CfgEditor::editFile(QString fileName,
                        QString sectionName,
                        FieldsDataList fields_data,
                        bool withKey)
{
    fields_data_ = fields_data;

    QFile file(fileName);
    file.open(QFile::ReadWrite);

    QDomDocument xml;
    if (!xml.setContent(&file))
        return;

    QDomElement elem1 = xml.documentElement();
    QDomNode node2 = elem1.firstChild();
    if (node2.hasChildNodes())
    {
        while (!node2.isNull())
        {
            QString foo1 = node2.nodeName();
            if (node2.nodeName() == sectionName)
            {
                QDomNode node3 = node2.firstChild();
                QString foo2 = node3.nodeName();
                if (withKey)
                {
                    bool targetKey = (node3.toElement().nodeName() == fields_data[0].first);
                    bool targetVal = (node3.toElement().firstChild().nodeValue() == fields_data[0].second);
                    if (targetKey && targetVal)
                    {
                        setNodeValues_(node3);
                    }
                }
                else
                {
                    setNodeValues_(node3);
                }
            }
            node2 = node2.nextSibling();
        }
    }

    file.close();

    QFile fn(fileName);
    if (fn.open(QFile::WriteOnly))
    {
        fn.write(xml.toByteArray(4));
    }
    fn.close();
}

//-----------------------------------------------------------------------------
//  Установить значения
//-----------------------------------------------------------------------------
void CfgEditor::setNodeValues_(QDomNode node3)
{
    while (!node3.isNull()) {
        if (node3.hasChildNodes())
        {
            QDomElement elem = node3.toElement();
            QString foo3 = elem.tagName();
            for (int i = 0, n = fields_data_.size(); i < n; ++i)
            {
                QString foo4 = fields_data_[i].first;
                QString foo5 = elem.nodeName();
                if (fields_data_[i].first == elem.nodeName())
                    elem.firstChild().setNodeValue(fields_data_[i].second.toString());
            }
        }
        node3 = node3.nextSibling();
    }
}






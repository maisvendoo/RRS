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

#ifndef CFGEDITOR_H
#define CFGEDITOR_H

#include <QXmlStreamWriter>
#include <QFile>
#include <QDomNode>

#include <QList>


#if defined(CFG_EDITOR_LIB)
# define CFGEDITOR_EXPORT Q_DECL_EXPORT
#else
# define CFGEDITOR_EXPORT Q_DECL_IMPORT
#endif


/// Структура входных данных для заполнения XML-файла
typedef QList<QPair<QString, QVariant>> FieldsDataList;


/*!
 *  \class CfgEditor
 *  \brief Класс содержит функциональность для обработки конфигом формата XML
 */
class CFGEDITOR_EXPORT CfgEditor
{
public:
    CfgEditor();

    /// Открыть файл для записи
    void openFileForWrite(QString fileName);
    /// Записать в файл список тегов
    void writeFile(FieldsDataList fields_data);
    /// Записать в файл в ноду <sectionName> список тегов
    void writeFile(QString sectionName, FieldsDataList fields_data);
    /// Закрыть файл по завершении записи
    void closeFileAfterWrite();

    /// Задать формат отступа: число больше нуля - количество пробелов
    /// или меньше нуля - количество табуляций
    void setIndentationFormat(int numSpacesOrTabs);

    /// Редактировать файл
    void editFile(QString fileName,
                 QString sectionName,
                 FieldsDataList fields_data,
                 bool withKey = false);


private:
    /// Описатель XML-файла
    QFile file_;
    /// Обеспечивает запись XML файла
    QXmlStreamWriter xmlWriter_;

    //
    FieldsDataList fields_data_;

    /// Установить значения нод
    void setNodeValues_(QDomNode node3);


};

#endif // CFGEDITOR_H

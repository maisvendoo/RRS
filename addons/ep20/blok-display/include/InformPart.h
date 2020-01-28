//-----------------------------------------------------------------------------
//
//      Элемент блока информации
//      (c) РГУПС, ВЖД 12/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 12/04/2017
 */

#ifndef INFORMPART_H
#define INFORMPART_H


#include <QLabel>
#include <QPainter>
#include <QImage>


/*!
 * \class InformPart
 * \brief Класс, описывающий элемент блока информации
 */
class InformPart : public QLabel
{
    Q_OBJECT

public:
    /*!
     * \brief Конструктор. Рисуем элемент информационного блока
     * \param geo - геометрия элемента (позиция, размер)
     * \param strHead - текст заголовка
     * \param strText - текст
     * \param align - выравнивание
     * \param marginLeft - отступ текста слева
     * \param isDrawX - рисовать ли крестик
     * \param isDrawO - рисовать ли эллипс
     */
    InformPart( QRect geo, QString strHead, QString strText,
                Qt::Alignment align, QWidget *parent = Q_NULLPTR,
                QString config_path = "",
                int marginLeft =0, bool isDrawX =false, bool isDrawO =false );
    /// Деструктор
    ~InformPart();

    /*!
     * \brief Установить текст элемента
     * \param strText - текст
     */
    void setText(QString strText);

    void setTextOverHead(QString headText, QRect geo);

    void setImgOverLabel();

private:

    QLabel* labelText_; ///< виждет для отображения текста элемента
    QLabel* labelTextOverHead_; ///< перекрывающий текст заголовка
    int fooWidth_;

    void drawX(); ///< Нарисовать крестик
    void drawO(); ///< Нарисовать эллипс

    // --- cfg параметры --- //
    QString colorBorder_;
    QString colorTextHead_;
    QString colorText_;
    int fontSize_;
    // --------------------- //

    /// Чтение конфигураций
    bool loadInformPartCfg(QString cfg_path);
};

#endif // INFORMPART_H

//-----------------------------------------------------------------------------
//
//      Элемент 2 блока информации
//      (c) РГУПС, ВЖД 13/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 13/04/2017
 */

#ifndef INFORMPART2_H
#define INFORMPART2_H


#include <QLabel>
#include <QPainter>
#include <QImage>


/*!
 * \class InformPart
 * \brief Класс, описывающий элемент блока информации
 */
class InformPart2 : public QLabel
{
    Q_OBJECT

public:
    /*!
     * \brief Конструктор. Рисуем элементы информационного блока
     * \param geo - геометрия элемента (позиция, размер)
     * \param strHead - текст заголовка
     */
    InformPart2(QRect geo, QString strHead, QWidget *parent = Q_NULLPTR );
    /// Деструктор
    ~InformPart2();

    /// Установить текст элемента
    void setVal(double strVal);

private:

    QLabel* labelVal_; ///< виджет для отображения значения элемента
    QLabel* labelBar_; ///< виджет для отображения прогресс бара
    QImage imgBar_;    ///< картинка для прогресс бара

    /// Установить виджет для отображения текста
    void setTextLabel_(QLabel* label, QRect geo, QColor color, QString str );

    /// Нарисовать тело элемента
    void drawBodyLabel_();


};



#endif // INFORMPART2_H

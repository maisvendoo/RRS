//-----------------------------------------------------------------------------
//
//      Элемент блока информации (треугольник)
//      (c) РГУПС, ВЖД 14/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации (треугольник)
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 14/04/2017
 */

#ifndef INFORMPARTTRIANGLE_H
#define INFORMPARTTRIANGLE_H

#include <QLabel>
#include <QPainter>
#include <QImage>


/*!
 * \class InformPartTriangle
 * \brief Класс, описывающий элемент блока информации (треугольник)
 */
class InformPartTriangle : public QLabel
{
    Q_OBJECT

public:
    /*!
     * \brief Конструктор. Рисуем элемент информационного блока (треугольник)
     * \param pos - позиция элемента
     * \param size - размер элемента
     * \param color - цвет элемента
     */
    InformPartTriangle( QPoint pos, int size, QColor color,
                        QWidget *parent = Q_NULLPTR);
    /// Деструктор
    ~InformPartTriangle();

    /*!
     * \brief Прорисовка треугольника
     * \param angle - установить угол поворота треугольника
     * \param isBrush - заполнять ли треугольник цветом
     * \param drawCircle - рисовать ли кругом внутри
     */
    void setTriangle(int angle, bool isBrush,
                     bool drawCircle = false, bool drawStr = false);


private:

    //QLabel* labelTriangle_; ///< виджет для отображения треугольника
    QImage imgTriangle_; ///< картинка для треугольника
    QColor colorTriangle_; ///< цвет треугольника


    /*!
     * \brief Нарисовать треугольник
     * \param paint - живописец
     * \param i, j - для определения вершин треугольника
     */
    void drawTriangle_(QPainter &paint, int i, int j);

    /*!
     * \brief Нарисовать круг в треугольнике
     * \param paint - живописец
     */
    void drawCircle_(QPainter &paint);

    /*!
     * \brief Нарисовать текст в треугольнике
     * \param paint - живописец
     * \param str - текст
     */
    void drawText(QPainter &paint, QString str);


};

#endif // INFORMPARTTRIANGLE_H

//-----------------------------------------------------------------------------
//
//      Спидометр. Шкала
//      (c) РГУПС, ВЖД 03/05/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Шкала" спидометра
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 03/05/2017
 */

#ifndef SSCALE_H
#define SSCALE_H

#include <QLabel>
#include <QPainter>
#include <QtMath>

/*!
 * \class SScale
 * \brief Класс, описывающий шкалу спидометра
 */
class SScale : public QLabel
{

public:

    /*!
     * \brief Конструктор.
     * \param valVMax - максимальное значение скорости спидометра
     * \param valVStep - шаг скорости
     * \param arcSplitAngle - угол центра разрыва дуги
     * \param arcSplitPoint - длина дуги-разрыва
     */
    SScale(int valVMax, int valVStep, int arcSplitAngle, int arcSplitPoint,
            QWidget *parent = Q_NULLPTR , QString config_dir = "");
    /// Деструктор
    ~SScale();

private:

    /// Рисуем шкалу спидометра
    void drawSScale_();

    // --- элементы шкалы спидометра --- //
    /// Второстепенные черточки
    void drawMinorLines_();
    /// Точки на шкале
    void drawPoints_();
    /// Значения скоростей
    void drawValV_();
    /// Основная дуга
    void drawArcMain_();
    /// Центральный круг
    void drawCircleCener_();
    // --------------------------------- //

    QPainter paint_; ///< рисовальщик

    double w_2_; ///< половина ширины класса
    double h_2_; ///< половина высоты класса

    /// Кол-во основных черточек на шкале спидометра
    int countLines_; ///< = максимальное_значение_скорости/шаг
    /// Шаг угла
    double aStep_;
    /// Угол стартовой точки (0 - начало шкалы спидометра)
    double aZeroPoint_;


    int valV_max_;       ///< макс. скорость
    int valV_step_;      ///< шаг скорости
    int arc_splitPoint_; ///< угол центра разрыва дуги
    int arc_splitAngle_; ///< длина дуги-разрыва


    // --- cfg параметры --- //
    /// второстепенные черточки на шкале спидометра
    bool    minorLine_isDraw_;  ///< рисовать ли черточки
    QString minorLine_color_;   ///< цвет
    int     minorLine_width_;   ///< ширина
    int     minorLine_length_;  ///< длина
    int     minorLine_count_;   ///< кол-во черточек между основными черточками

    /// вспомогательные точки на шкале спидометра
    bool    points_isDraw_; ///< рисовать ли точки на спидометре
    QString points_color_;  ///< цвет
    int     points_width_;  ///< диаметр

    /// значения скоростей на шкале спидометра
    QString valV_color_;    ///< цвет
    int     valV_fontSize_; ///< размер шрифта
    int     valV_margin_;   ///< отступ от края дуги

    /// дуга шкалы спидометра
    QString arc_color_;         ///< цвет
    int     arc_width_;         ///< ширина

    /// центральный круг спидометра
    QString circleCener_color_; ///< цвет
    int     circleCener_width_; ///< диаметр
    // --------------------- //


    /// Чтение конфигураций
    bool loadSScaleCfg(QString cfg_path);


};

#endif // SSCALE_H

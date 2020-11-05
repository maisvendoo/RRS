//-----------------------------------------------------------------------------
//
//      Спидометр. Дуга ограничения
//      (c) РГУПС, ВЖД 31/03/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Дуга ограничения" спидометра
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 04/05/2017
 */

#ifndef SARCLIMIT_H
#define SARCLIMIT_H

#include <QLabel>
#include <QPainter>

/*!
 * \class SArcLimit
 * \brief Прорисовка дуги ограничения на шкале спидометра
 */
class SArcLimit : public QLabel
{

public:

    /*!
     * \brief Конструктор.
     * \param valVMax - максимальное значение скорости спидометра
     * \param arcSplitAngle - угол центра разрыва дуги
     * \param arcSplitPoint - длина дуги-разрыва
     */
    SArcLimit(int valVMax, int arcSplitAngle, int arcSplitPoint,
               QWidget *parent = Q_NULLPTR , QString config_dir = "");
    /// Деструктор
    ~SArcLimit();

    /*!
     * \brief Нарисовать дуги ограничений скорости
     * \param valSpeedLimit - значение текущего ограничения
     * \param nextValSpeedLimit - значение следующего ограничения
     */
    void setValSpeedLimit(double curSpeedLimit, double nextSpeedLimit);

    /// Получить ширину дуги ограничения скорости
    int getArcLimitWidth();

private:

    QImage img_; ///< картинка

    // --- cfg параметры --- //
    /// дуга ограничения шкалы спидометра
    int     arcLimit_width_;     ///< ширина
    QString arcLimit_color_;     ///< цвет
    QString arcLimitNext_color_; ///< цвет
    // --------------------- //

    int valVMax_;       ///< максимальное значение скорости спидометра
    int arcSplitAngle_; ///< угол центра разрыва дуги
    int arcSplitPoint_; ///< длина дуги-разрыва

    /// Чтение конфигураций
    bool loadSArcLimitCfg(QString cfg_path);

};

#endif // SARCLIMIT_H

//-----------------------------------------------------------------------------
//
//      Спидометр. Стрелка
//      (c) РГУПС, ВЖД 02/05/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Стрелка" спидометра
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 02/05/2017
 */

#ifndef SARROW_H
#define SARROW_H


#include <QLabel>
#include <QPainter>

/*!
 * \class SArrow
 * \brief Класс, описывающий стрелку спидометра
 */
class SArrow : public QLabel
{

public:
    /// Конструктор
    SArrow(QWidget *parent = Q_NULLPTR, QString config_dir = "");
    /// Деструктор
    ~SArrow();

    /// Вращение стрелки
    void setValSpeed(double angle);


private:
    QImage   img_; ///< картинка стрелки спидометра

    double w_2_; ///< половина ширины класса
    double h_2_; ///< половина высоты класса

    // --- cfg параметры --- //
    /// стрелка спидометра
    double  kLength_;   ///< коэфициент длины стрелки (от ширины спидометра)
    double  width_;     ///< ширина основания стрелки
    QString color_;     ///< цвет стрелки

    // --------------------- //


    /// Чтение конфигураций
    bool loadSArrowCfg(QString cfg_path);

};



#endif // SARROW_H

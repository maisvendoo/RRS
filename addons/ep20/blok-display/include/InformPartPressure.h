//-----------------------------------------------------------------------------
//
//      Элемент-давления блока информации
//      (c) РГУПС, ВЖД 15/01/2020
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента-давления блока информации
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 15/01/2020
 */


#ifndef INFORMPARTPRESSURE_H
#define INFORMPARTPRESSURE_H

#include <QLabel>
#include <QPainter>
#include <QImage>
#include "ImageLabel.h"

class InformPartPressure : public ImageLabel
{
public:
    InformPartPressure(QString strHead, QWidget *parent = Q_NULLPTR);

    void setValPressure(double val);

private:
    ImageLabel* labelPressureBar_;
    QLabel* labelPressureVal_;

    QImage img_;
    QPixmap pm;

    void drawLabel(QString val, int posY);

};

#endif // INFORMPARTPRESSURE_H

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

#include "InformPartPressure.h"


//-----------------------------------------------------------------------------
//  Конструктор
//-----------------------------------------------------------------------------
InformPartPressure::InformPartPressure(QString strHead, QWidget *parent)
    : ImageLabel(parent)
{
    this->setGeometry(0, 0, 70, 550);
    this->setStyleSheet("border: 2px solid yellow;"
                        "color: white;"
                        "font-weight: bold;");

    // --- ВЕРХНЯЯ ЧАСТЬ ЭЛЕМЕНТА --- //
    QLabel* labelTop = new QLabel(this);
    labelTop->setGeometry(0, 0,
                          this->width(), this->height()*0.8);

    double deltaY = (labelTop->height()-20)/5;
    drawLabel("1.00", 3);
    drawLabel("0.80", deltaY);
    drawLabel("0.60", deltaY*2);
    drawLabel("0.40", deltaY*3);
    drawLabel("0.20", deltaY*4);
    drawLabel("0.00", deltaY*5);

    QImage img(QSize(labelTop->width(), labelTop->height() - 10),
               QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter paint(&img);

    paint.setBrush(QColor(255, 255, 255));
    QPolygon rect;
    rect << QPoint(37, 0)
         << QPoint(37, labelTop->height() - 13)
         << QPoint(60, labelTop->height() - 13)
         << QPoint(60, 0);
    paint.drawPolygon(rect);

    paint.setPen(QColor(0, 0, 255));
    double deltaBarY = (labelTop->height() - 13)/10.0;
    for (int i = 1; i < 10; ++i)
    {
        paint.drawLine((i%2 == 0) ? (38) : (50), deltaBarY*i,
                       59, deltaBarY*i);
    }

    paint.end();
    labelTop->setPixmap(QPixmap::fromImage(img));

    //
    labelPressureBar_ = new ImageLabel(this);
    labelPressureBar_->setGeometry(labelTop->geometry());

    img_ = QImage(QSize(labelTop->width(), labelTop->height() - 10),
                  QImage::Format_ARGB32_Premultiplied);






    // --- НИЖНЯЯ ЧАСТЬ ЭЛЕМЕНТА --- //
    QLabel* labelBottom = new  QLabel(this);
    labelBottom->setGeometry(0, labelTop->height(),
                             this->width(), this->height()*0.2);
    labelBottom->setFont(QFont("Arial", 16));
    labelBottom->setStyleSheet("color: yellow");
    labelBottom->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    labelBottom->setText(strHead);

    labelPressureVal_ = new QLabel("0,0", this);
    labelPressureVal_->setGeometry(labelBottom->geometry());
    labelPressureVal_->setFont(QFont("Arial", 16));
    labelPressureVal_->setAlignment(Qt::AlignCenter);

    QLabel* label_MPa = new QLabel("МПа", this);
    label_MPa->setGeometry(labelBottom->geometry());
    label_MPa->setFont(QFont("Arial", 16));
    label_MPa->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

}



//-----------------------------------------------------------------------------
//  Установить значение давления
//-----------------------------------------------------------------------------
void InformPartPressure::setValPressure(double val)
{
    labelPressureVal_->setText(QString("%1").arg(val, 4, 'f', 2).replace('.', ','));

    img_.fill(Qt::transparent);
    QPainter paint(&img_);
    paint.setPen(Qt::NoPen);
    paint.setBrush(QColor(0, 150, 255));
    double pressureBarH = labelPressureBar_->height() - 13;
    QPolygon rect;
    rect << QPoint(43, (pressureBarH)*(1.0 - val))
         << QPoint(43, pressureBarH)
         << QPoint(54, pressureBarH)
         << QPoint(54, (pressureBarH)*(1.0 - val));
    paint.drawPolygon(rect);
    paint.end();
    pm = QPixmap::fromImage(img_);
    labelPressureBar_->setPixmap(&pm);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InformPartPressure::drawLabel(QString val, int posY)
{
    QLabel* label = new QLabel(val, this);
    label->setFont(QFont("Arial", 10));
    label->setStyleSheet("border: none");
    label->move(7, posY);

}



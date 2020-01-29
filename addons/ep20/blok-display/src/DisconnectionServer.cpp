//-----------------------------------------------------------------------------
//
//      Экран потери связи
//      (c) РГУПС, ВЖД 18/07/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Экран потери связи
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 18/07/2017
 */

#include "DisconnectionServer.h"


//-----------------------------------------------------------------------------
//  Конструктор
//-----------------------------------------------------------------------------
DisconnectionServer::DisconnectionServer(QSize size, QWidget *parent)
    : QLabel(parent)
{
    this->resize(size);


    QImage imgScreen(this->size(), QImage::Format_ARGB32_Premultiplied);
    imgScreen.fill(Qt::transparent);

    QPainter paint(&imgScreen);


    int fooX0 = this->width()/4;
    int fooY0 = this->height()/4;
    int fooPadd = 4;
    int fooW = this->width()/2;
    int fooH = this->height()/2;

    // Рисуем прямоугольники
    paint.setBrush(Qt::black);
    paint.drawRect(-1, -1,
                   this->width(), this->height());

    paint.setBrush(Qt::red);
    paint.drawRect(fooX0 - 1, fooY0 - 1,
                   fooW, fooH);

    paint.setBrush(Qt::black);
    paint.drawRect(fooX0 + fooPadd - 1, fooY0 + fooPadd - 1,
                   fooW - fooPadd*2, fooH - fooPadd*2);

    paint.setBrush(Qt::red);
    paint.drawRect(fooX0 + fooPadd*2 - 1, fooY0 + fooPadd*2 - 1,
                   fooW - fooPadd*4, fooH - fooPadd*4);

    // Выводим надпись
    paint.setFont(QFont("Arial", 22, 86));
    paint.drawText(fooW - 170 , fooH, QString("Потеряна связь с КЛУБ"));


    paint.end();

    this->setPixmap(QPixmap::fromImage(imgScreen));

}

//-----------------------------------------------------------------------------
//  Деструктор
//-----------------------------------------------------------------------------
DisconnectionServer::~DisconnectionServer()
{

}




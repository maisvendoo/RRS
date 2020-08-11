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

#include "SArrow.h"
#include "CfgReader.h"
#include "qmath.h"

const	QString		SARROW_CFG = "../cfg/BLOK/SArrow.xml";


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SArrow::SArrow(QWidget *parent, QString config_dir) : QLabel(parent)
{
    this->resize( parent->size() );
    w_2_ = this->width()/2.0;
    h_2_ = this->height()/2.0;

    // загружаем параметры с конфига
    loadSArrowCfg(config_dir + "SArrow.xml");

    img_ = QImage( this->size(),
                   QImage::Format_ARGB32_Premultiplied );

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SArrow::~SArrow()
{

}

//-----------------------------------------------------------------------------
//  Вращение стрелки
//-----------------------------------------------------------------------------
void SArrow::setValSpeed(double angle)
{
    img_.fill(Qt::transparent);

    QPainter paint(&img_);
    // указываем цвет границы
    paint.setPen(QPen(QColor(color_), 1, Qt::SolidLine));
    // указываем цвет заливки
    paint.setBrush(QColor(color_));
    // повышаем качество рисовки
    paint.setRenderHint(QPainter::Antialiasing, true);


              angle = qDegreesToRadians(angle);
    double fooAngle = qDegreesToRadians(90.0);
    // длина стрелки
    double r  = kLength_;
    // половина ширины основания стрелки
    double r2 = width_;

    QPolygonF triangle;
    triangle << QPointF( w_2_ + (w_2_*r)*cos(angle),
                         h_2_ + (h_2_*r)*sin(angle) )
             << QPointF( w_2_ + r2*cos(angle+fooAngle),
                         h_2_ + r2*sin(angle+fooAngle) )
             << QPointF( w_2_ + r2*cos(angle-fooAngle),
                         h_2_ + r2*sin(angle-fooAngle) );

    paint.drawPolygon(triangle);
    paint.end();

    this->setPixmap(QPixmap::fromImage(img_));

}

//-----------------------------------------------------------------------------
//  Чтение конфига
//-----------------------------------------------------------------------------
bool SArrow::loadSArrowCfg(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString sectionName = "SArrow";

        // коэфициент длины стрелки (от ширины спидометра)
        if (!cfg.getDouble(sectionName, "koef_length", kLength_))
            kLength_ = 0.8;
        // ширина основания стрелки
        if (!cfg.getDouble(sectionName, "width", width_))
            width_ = 6.0;
        // цвет стрелки спидометра
        if (!cfg.getString(sectionName, "color", color_))
            color_ = "#ffffff";

    }
    else
    {
        // Параметры по умолчанию.
        // стрелка спидометра
        kLength_ = 0.8;     // коэфициент длины стрелки (от ширины спидометра)
        width_ = 6.0;       // ширина основания стрелки
        color_ = "#ffffff"; // цвет стрелки спидометра

    }

    return true;
}

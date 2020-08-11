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

#include "SScale.h"
#include "CfgReader.h"
#include "math.h"

const	QString		SSCALE_CFG = "../cfg/BLOK/SScale.xml";


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SScale::SScale( int valVMax, int valVStep, int arcSplitAngle, int arcSplitPoint,
                QWidget *parent, QString config_dir ) : QLabel(parent)
{
    this->resize( parent->size() );
    w_2_ = this->width()/2.0;
    h_2_ = this->height()/2.0;

    // установить максимальное значение скорости спидометра
    this->valV_max_ = valVMax;
    // установить шаг значения скоростей на шкале спидометра
    this->valV_step_ = valVStep;
    // установить угол центра разрыва дуги
    this->arc_splitAngle_ = arcSplitAngle;
    // установить длину дуги-разрыва
    this->arc_splitPoint_ = arcSplitPoint;

    /********************************************
    *   форумулы нахождения точки на окружности
    *   x = x0 + r*cos(a)
    *   y = y0 + r*sin(a)
    ********************************************/

    // загружаем параметры с конфига
    loadSScaleCfg(config_dir + "SScale.xml");

    // округляем значение макс. скорости на велечину шага
    if ( valV_max_ % valV_step_ != 0 )
        valV_max_ = valV_max_ + valV_step_ - (valV_max_ % valV_step_);

    // кол-во основных черточек
    countLines_ = valV_max_/valV_step_;                     // Богос. вытащить
    // шаг угла
    aStep_ = (360.0 - arc_splitAngle_)/countLines_;     // Богос. вытащить
    // угол стартовой точки (0 - начало шкалы спидометра)
    aZeroPoint_ = arc_splitPoint_ - arc_splitAngle_/2;  // Богос. вытащить

    // рисуем шкалу спидометра
    drawSScale_();

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SScale::~SScale()
{

}

//-----------------------------------------------------------------------------
//  Рисуем шкалу спидометра
//-----------------------------------------------------------------------------
void SScale::drawSScale_()
{
    // создаем img размером с widget
    QImage img = QImage( this->size(),
                         QImage::Format_ARGB32_Premultiplied );
    // делаем его прозрачным
    img.fill(Qt::transparent);
    // создаем и загружаем pixmap с картинки img
    QPixmap pix = QPixmap::fromImage(img);
    // передаем ссылку на pixmap в QPainter
    paint_.begin(&pix);
    // повышаем качество рисовки
    paint_.setRenderHint(QPainter::Antialiasing, true);


    // --- рисуем элементы шкалы спидометра --- //

    // рисуем второстепенные черточки
    if (minorLine_isDraw_)
        drawMinorLines_();
    // рисуем точки
    if (points_isDraw_)
        drawPoints_();
    // рисуем значения скоростей
    drawValV_();
    // рисуем основную дугу шкалы спидометра
    drawArcMain_();
    // рисуем центральный круг спидометра
    drawCircleCener_();

    // ---------------------------------------- //

    paint_.end();
    this->setPixmap(pix);

}

//-----------------------------------------------------------------------------
//  Второстепенные черточки
//-----------------------------------------------------------------------------
void SScale::drawMinorLines_()
{
    for (int i = 0; i < countLines_; i++)
    {

        for (int j = 1; j < (minorLine_count_ + 1); j++)
        {
            double angle = i*aStep_ +
                           j*aStep_/(minorLine_count_ + 1) -
                           aZeroPoint_;
            angle = qDegreesToRadians(angle);

            // радиус круга
            double r  = - 31 - (26 - minorLine_length_)/2; // Богос (26 = "majorLine_length")
            // радиус "внутреннего" круга
            double r2  = - 31 - (26 + minorLine_length_)/2; // Богос (26 = "majorLine_length")

            // 1. указываем цвет, толщину и стиль линии
            paint_.setPen(QPen( QColor(minorLine_color_),
                                minorLine_width_,
                                Qt::SolidLine,
                                Qt::RoundCap ));

            // 2. рисуем линию
            paint_.drawLine( w_2_ + (w_2_ + r)*cos(angle),
                             h_2_ + (h_2_ + r)*sin(angle),
                             w_2_ + (w_2_ + r2)*cos(angle),
                             h_2_ + (h_2_ + r2)*sin(angle) );

        }
    }
}

//-----------------------------------------------------------------------------
//  Точки на шкале
//-----------------------------------------------------------------------------
void SScale::drawPoints_()
{
    for (int i = 0; i < countLines_; i++)
    {
        double angle = (i + 0.5)*aStep_ - aZeroPoint_;
        angle = qDegreesToRadians(angle);

        // радиус круга
        double r  = - 20 - (26 + minorLine_length_); // Богос (26 = "majorLine_length")

        // 1. указываем цвет, толщину и стиль линии
        paint_.setPen(QPen( QColor(points_color_),
                            points_width_,
                            Qt::SolidLine,
                            Qt::RoundCap ));

        // 2. рисуем линию
        paint_.drawPoint( w_2_ + (w_2_ + r)*cos(angle),
                          h_2_ + (h_2_ + r)*sin(angle) );
    }

}

//-----------------------------------------------------------------------------
//  Значения скоростей
//-----------------------------------------------------------------------------
void SScale::drawValV_()
{
    double r  = - 10 - valV_margin_;

    // 1. указываем цвет текста
    paint_.setPen(QPen(QColor(valV_color_)));

    // 2. указываем шрифт, размер и "жирность"
    paint_.setFont(QFont("Arial", valV_fontSize_, 75));

    // 3. рисуем текст
    for (int i = 0; i <= countLines_; i++)
    {
        double angle = i*aStep_ - aZeroPoint_;
        angle = qDegreesToRadians(angle);

        paint_.drawText( (w_2_ - 20) + (w_2_ + r)*cos(angle),
                         (h_2_ + 10) + (h_2_ + r)*sin(angle),
                         QString::number(i*valV_step_) );
    }

}


//-----------------------------------------------------------------------------
//  Основная дуга
//-----------------------------------------------------------------------------
void SScale::drawArcMain_()
{
    paint_.setPen(QPen( QColor(arc_color_),
                        arc_width_,
                        Qt::SolidLine ));
    QRectF rectangle(30, 30, this->width()-60, this->height()-60);
    paint_.drawArc( rectangle,
                    (arc_splitPoint_ + arc_splitAngle_/2 + arc_width_/8)*16,
                    (360 - arc_splitAngle_ - arc_width_/8*2)*16 );

}

//-----------------------------------------------------------------------------
//  Центральный круг
//-----------------------------------------------------------------------------
void SScale::drawCircleCener_()
{
    paint_.setPen(QPen( QColor(circleCener_color_),
                        circleCener_width_,
                        Qt::SolidLine,
                        Qt::RoundCap ));
    paint_.drawPoint( this->width()/2,
                      this->height()/2 );

}


//-----------------------------------------------------------------------------
//  Чтение конфига
//-----------------------------------------------------------------------------
bool SScale::loadSScaleCfg(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString sectionName = "SScale";

        // --- второстепенные черточки на шкале спидометра --- //
        // рисовать ли черточки
        if (!cfg.getBool(sectionName, "minorLine_isDraw", minorLine_isDraw_))
            minorLine_isDraw_ = true;
        // цвет
        if (!cfg.getString(sectionName, "minorLine_color", minorLine_color_))
            minorLine_color_ = "#ffffff";
        // ширина
        if (!cfg.getInt(sectionName, "minorLine_width", minorLine_width_))
            minorLine_width_ = 2;
        // длина
        if (!cfg.getInt(sectionName, "minorLine_length", minorLine_length_))
            minorLine_length_ = 10;
        // количество черточек между основными черточками
        if (!cfg.getInt(sectionName, "minorLine_count", minorLine_count_))
            minorLine_count_ = 4;

        // --- вспомогательные точки на шкале спидометра --- //
        // рисовать ли точки на спидометре
        if (!cfg.getBool(sectionName, "points_isDraw", points_isDraw_))
            points_isDraw_ = false;
        // цвет
        if (!cfg.getString(sectionName, "points_color", points_color_))
            points_color_ = "#ffffff";
        // диаметр
        if (!cfg.getInt(sectionName, "points_width", points_width_))
            points_width_ = 5;

        // --- значения скоростей на шкале спидометра --- //
        // цвет
        if (!cfg.getString(sectionName, "valV_color", valV_color_))
            valV_color_ = "#ffffff";
        // размер шрифта
        if (!cfg.getInt(sectionName, "valV_fontSize", valV_fontSize_))
            valV_fontSize_ = 20;
        // отступ от края дуги
        if (!cfg.getInt(sectionName, "valV_margin", valV_margin_))
            valV_margin_ = 70;

        // --- дуга шкалы спидометра --- //
        // цвет
        if (!cfg.getString(sectionName, "arc_color", arc_color_))
            arc_color_ = "#ffffff";
        // ширина
        if (!cfg.getInt(sectionName, "arc_width", arc_width_))
            arc_width_ = 4;

        // --- центральный круг спидометра --- //
        // цвет
        if (!cfg.getString(sectionName, "circleCener_color", circleCener_color_))
            circleCener_color_ = "#ffffff";
        // диаметр
        if (!cfg.getInt(sectionName, "circleCener_width", circleCener_width_))
            circleCener_width_ = 70;

    }
    else
    {
        // Параметры по умолчанию.

        // второстепенные черточки на шкале спидометра
        minorLine_isDraw_ = true;       // рисовать ли черточки
        minorLine_color_ = "#ffffff";   // цвет
        minorLine_width_ = 2;           // ширина
        minorLine_length_ = 10;         // длина
        minorLine_count_ = 4;           // кол-во черточек

        // вспомогательные точки на шкале спидометра
        points_isDraw_ = false;     // рисовать ли точки на спидометре
        points_color_ = "#ffffff";  // цвет
        points_width_ = 5;          // диаметр

        // значения скоростей на шкале спидометра
        valV_color_ = "#ffffff";    // цвет
        valV_fontSize_ = 20;        // размер шрифта
        valV_margin_ = 70;          // отступ от края дуги

        // дуга шкалы спидометра --- //
        arc_color_ = "#ffffff"; // цвет
        arc_width_ = 4;         // ширина


        // центральный круг спидометра
        circleCener_color_ = "#ffffff"; // цвет
        circleCener_width_ = 70;        // диаметр

    }

    return true;
}

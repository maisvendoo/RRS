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

#include "SArcLimit.h"
#include "CfgReader.h"
#include "qmath.h"

const	QString		SARCLIMIT_CFG = "../cfg/BLOK/SArcLimit.xml";

const   double      EPS = 0.0001;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SArcLimit::SArcLimit( int valVMax, int arcSplitAngle, int arcSplitPoint,
                      QWidget *parent, QString config_dir ) : QLabel(parent)
{
    this->resize(parent->size());

    // установить максимальное значение скорости спидометра
    this->valVMax_ = valVMax;
    // установить угол центра разрыва дуги
    this->arcSplitAngle_ = arcSplitAngle;
    // установить длину дуги-разрыва
    this->arcSplitPoint_ = arcSplitPoint;

    // загружаем параметры с конфига
    loadSArcLimitCfg(config_dir + "SArcLimit.xml");

    // создаем img размером с widget
    img_ = QImage( this->size(),
                   QImage::Format_ARGB32_Premultiplied );

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SArcLimit::~SArcLimit()
{

}

//-----------------------------------------------------------------------------
//  Нарисовать дугу ограничения скорости
//-----------------------------------------------------------------------------
void SArcLimit::setValSpeedLimit(double curSpeedLimit, double nextSpeedLimit)
{ // Богос. Требуется рефакторинг.
    curSpeedLimit += 2;

    img_.fill(Qt::transparent);
    // создаем и загружаем pixmap с картинки img_
    QPixmap pixSpeedLimit = QPixmap::fromImage(img_);
    // передаем ссылку на pixmap в QPainter
    QPainter paintSL(&pixSpeedLimit);
    // повышаем качество рисовки
    paintSL.setRenderHint(QPainter::Antialiasing, true);


    QRectF rectangleSpeedLimit(30, 30, this->width()-60, this->height()-60);

    // значение угла максимальной скорости на шкале
    double foo_a_maxSpeed = arcSplitPoint_ + arcSplitAngle_/2;
    // значение шага скорости дуги шкалы (т.е. сколько градусов в 1 км/ч?)
    double foo_a_step = (360.0 - arcSplitAngle_)/valVMax_;

    // значение угла значения ограничения
    double foo_a = foo_a_maxSpeed /*+ arcLimit_width_/8*/ +
                   (valVMax_ - curSpeedLimit)*foo_a_step;


    // --- рисуем желтую дугу ограничения --- //
    double foo_aLen = 0; // значение длины дуги ограничения
    if (nextSpeedLimit < curSpeedLimit)
        foo_aLen = (curSpeedLimit - nextSpeedLimit)*foo_a_step - arcLimit_width_/8/**2*/;
    else
        foo_aLen =                              (2)*foo_a_step - arcLimit_width_/8/**2*/;

    paintSL.setPen(QPen( QColor(arcLimitNext_color_),
                         arcLimit_width_,
                         Qt::SolidLine ));
    paintSL.drawArc( rectangleSpeedLimit,
                     (foo_a)*16,
                     (foo_aLen)*16 );
    // -------------------------------------- //

    // --- рисуем красную дугу ограничения --- //
    paintSL.setPen(QPen( QColor(arcLimit_color_),
                         arcLimit_width_,
                         Qt::SolidLine ));
    paintSL.drawArc( rectangleSpeedLimit,
                     (foo_a_maxSpeed + arcLimit_width_/8)*16,
                     ( 360 - arcSplitAngle_ - arcLimit_width_/8*2 -
                       curSpeedLimit*foo_a_step )*16 );
    // --------------------------------------- //

    paintSL.end();

    this->setPixmap(pixSpeedLimit);

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int SArcLimit::getArcLimitWidth()
{
    return this->arcLimit_width_;
}

//-----------------------------------------------------------------------------
//  Чтение конфигураций дуги ограничения шкалы спидометра
//-----------------------------------------------------------------------------
bool SArcLimit::loadSArcLimitCfg(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

        QString sectionName = "SArcLimit";

        // --- параметры дуги ограничения шкалы спидометра --- //
        // ширина линии
        if (!cfg.getInt(sectionName, "arcLimit_width", arcLimit_width_))
            arcLimit_width_ = 4;
        // цвет
        if (!cfg.getString(sectionName, "arcLimit_color", arcLimit_color_))
            arcLimit_color_ = "#ff3232";
        // цвет
        if (!cfg.getString(sectionName, "arcLimitNext_color", arcLimitNext_color_))
            arcLimitNext_color_ = "#ffff50";


    }
    else
    {
        // Параметры по умолчанию.

        // параметры дуги ограничения шкалы спидометра
        arcLimit_width_ = 4;             // ширина линии
        arcLimit_color_ = "#ff3232";     // цвет
        arcLimitNext_color_ = "#ffff50"; // цвет

    }

    return true;
}

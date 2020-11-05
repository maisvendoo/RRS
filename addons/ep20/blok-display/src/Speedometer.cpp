//-----------------------------------------------------------------------------
//
//      Спидометр
//      (c) РГУПС, ВЖД 31/03/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента "Спидометр"
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 31/03/2017
 */

#include "Speedometer.h"
#include "CfgReader.h"
#include "cmath"

using namespace std;

const	QString		SPEEDOMETER_CFG = "../cfg/BLOK/Speedometer.xml";
//const	QString		STRIANGLE_CFG   = "../cfg/BLOK/STriangle.xml";
const   double      EPS             = 0.0001;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Speedometer::Speedometer(QRect geo, QWidget *parent, QString config_dir) : QLabel(parent)
{
//    this->setAutoFillBackground(true);
//    this->setPalette(QPalette(QColor(150,255,150)));
//    this->setStyleSheet("border: 2px solid red");

    this->resize(geo.width(),geo.height());
    this->move(geo.left(), geo.top());

    // загружаем параметры с конфига
    loadSpeedometerCfg(config_dir + "Speedometer.xml");
//    loadSTriangleCfg(STRIANGLE_CFG);

    // инициализация переменных
    speedCurOld_i_    = -1; // предыдущая текущая скорость (int)
    speedCurOld_d_    = -1; // предыдущая текущая скорость (double)
    speedLimitOld_    = -1; // предыдущее ограничение скорости
//    speedRefOld_      = -1; // предыдущая заданная скорость с рукоятки
//    zzzOld_           = -1;

    timeCount_ = 0;
    timerForBlink_ = new QTimer();
    timerForBlink_->setInterval(500);
    connect(timerForBlink_, SIGNAL(timeout()), this, SLOT(onTimer()));


    // ШКАЛА
    sScale_ = new SScale( valV_max_, valV_step_,
                          arc_splitAngle_, arc_splitPoint_,
                          this, config_dir );

    // угол начальной точки (0 на шкале спидометра)     // Богос...
    aBeginPoint_ = 360.0 - arc_splitPoint_ + arc_splitAngle_/2;
    aStep_ = (360.0 - arc_splitAngle_)/valV_max_; // шаг угла



//    // ЖЕЛТЫЙ ТРЕУГОЛЬНИК
//    sTriangleYellow_ = new STriangle(this);
//    sTriangleYellow_->setParameters( widthTriangle_Yellow_,
//                                     lengthTriangle_Yellow_,
//                                     QColor(colorTriangle_Yellow_) );

//    // КРАСНЫЙ ТРЕУГОЛЬНИК
//    sTriangleRed_ = new STriangle(this);
//    sTriangleRed_->setParameters( widthTriangle_Red_,
//                                  lengthTriangle_Red_,
//                                  QColor(colorTriangle_Red_) );

    // ДУГА ОГРАНИЧЕНИЯ
    sArcLimit_ = new SArcLimit(valV_max_, arc_splitAngle_, arc_splitPoint_, this, config_dir);

    // ОСНОВНЫЕ ЛИНИИ ШКАЛЫ СПИДОМЕТРА
    sScaleMajorLine_ = new SScaleMajorLine( majorLine_length_,
                                            majorLine_width_,
                                            majorLine_color_,
                                            valV_max_/valV_step_,
                                            arc_splitPoint_/(valV_max_/valV_step_),
                                            arc_splitPoint_ - arc_splitAngle_/2,
                                            sArcLimit_->getArcLimitWidth(),
                                            this);

    // СТРЕЛКА
    sArrow_ = new SArrow(this, config_dir);


    // ТЕКУЩАЯ СКОРОСТЬ (в центре спидометра)
    labelValVCenter_ = new QLabel("0", this);
    // ОГРАНИЧЕНИЕ СКОРОСТИ (в центре спидометра)
    labelSpeedLimit_ = new QLabel("120", this);
    // устанавливаем положение label текущей скорости в центре спидометра
    myDrawValVCenter_( labelValVCenter_,
                       labelValVCenter_x_,
                       labelValVCenter_y_,
                       labelValVCenter_color_,
                       labelValVCenter_fontSize_ );
    // устанавливаем положение label ограничения скорости
    myDrawValVCenter_( labelSpeedLimit_,
                       labelSpeedLimit_x_,
                       labelSpeedLimit_y_,
                       labelSpeedLimit_color_,
                       labelSpeedLimit_fontSize_ );

    timerForBlink_->start();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Speedometer::~Speedometer()
{

}

//-----------------------------------------------------------------------------
//  Установить текущую скорость
//-----------------------------------------------------------------------------
void Speedometer::setSpeedCur(double speed)
{
    // чтобы не перерисовывать стрелку спидометра, если значение не изменилось
    if ( abs(speedCurOld_d_ - speed) < EPS )
        return;
    speedCurOld_d_ = speed;

    // рисуем значение текущей скорости стрелкой
    sArrow_->setValSpeed(aBeginPoint_ + setSpeedRange_(speed)*aStep_);

    // чтобы не перерисовывать знач. текущей скорости, если оно не изменилось
    if ( abs(speedCurOld_i_ - round(speed)) < EPS )
        return;
    speedCurOld_i_ = static_cast<int>(round(speed));


    // возможно надо будет убрать условия, которые ограничивают перерисовывание,
    // чтобы мигание "не испортить"/не прерывать. Богос


    // рисуем значение текущей скорости на центральном круге спидометра
    labelValVCenter_->setText( parsSpeedCur_(setSpeedRange_(speed)) );

}

//-----------------------------------------------------------------------------
//  Установить значение ограничения скорости
//-----------------------------------------------------------------------------
//void Speedometer::setSpeedLimit(int speedLimit)
//{
//    // чтобы не перерисовывать знач. огранич. скорости, если оно не изменилось
//    if ( abs(speedLimitOld_ - speedLimit) < EPS )
//        return;
//    speedLimitOld_ = speedLimit;

//    // рисуем ограничение скорости
//    labelSpeedLimit_->setText( parsSpeedCur_(setSpeedRange_(speedLimit)) );

//}

//-----------------------------------------------------------------------------
//  Установить ограничения скорости
//-----------------------------------------------------------------------------
void Speedometer::setSpeedLimits(int speedLimit, int nextSpeedLimit)
{
    // чтобы не перерисовывать, если значение не изменилось
    if ( abs(speedLimitOld_ - speedLimit) <= 1 )
        return;
    speedLimitOld_ = speedLimit;

    // рисуем значение ограничения скорости в центре спидометра
    labelSpeedLimit_->setText( parsSpeedCur_(setSpeedRange_(speedLimit)) );

    // рисуем текущее ограничение скорости дугой
    sArcLimit_->setValSpeedLimit( setSpeedRange_(speedLimit),
                                  setSpeedRange_(nextSpeedLimit) );

    // рисуем следующее ограничение скорости дугой
    sScaleMajorLine_->drawMajorLine();
    // zzzzzzzzzzzzzzzzzzzzzzzzzzzz
}

//-----------------------------------------------------------------------------
//  Моргание значения текущей скорости, при приближении к значению ограничения
//-----------------------------------------------------------------------------
void Speedometer::onTimer()
{
    // условие значения приближения
    if (speedLimitOld_ - speedCurOld_i_ <= 3)
    {
        timeCount_++;
        // "время ожидания" перед миганием
        if (timeCount_ > 2)
        {
            // процесс мигания
            labelValVCenter_->setVisible(!labelValVCenter_->isVisible());
        }
    }
    else
    {
        timeCount_ = 0;
        labelValVCenter_->setVisible(true);
    }
}

//-----------------------------------------------------------------------------
//  Установить заданную скорость с рукоятки
//-----------------------------------------------------------------------------
//void Speedometer::setSpeed_Yellow(int speedLimit_Yellow)
//{
//    // чтобы не перерисовывать, если значение не изменилось
//    if ( abs(speedRefOld_ - speedLimit_Yellow) < EPS )
//        return;
//    speedRefOld_ = speedLimit_Yellow;

//    sTriangleYellow_->setValSpeed(aBeginPoint_ +
//                                  setSpeedRange_(speedLimit_Yellow)*aStep_);
//}

//-----------------------------------------------------------------------------
//  Установить zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz
//-----------------------------------------------------------------------------
//void Speedometer::setSpeed_Red(int speedLimit_Red)
//{
//    // чтобы не перерисовывать, если значение не изменилось
//    if ( abs(zzzOld_ - speedLimit_Red) < EPS )
//        return;
//    zzzOld_ = speedLimit_Red;

//    sTriangleRed_->setValSpeed(aBeginPoint_ +
//                               setSpeedRange_(speedLimit_Red)*aStep_);
//}

//-----------------------------------------------------------------------------
//  Рисуем первоначальную скорость в центре спидометра
//-----------------------------------------------------------------------------
void Speedometer::myDrawValVCenter_(QLabel *label, int x, int y, QString color,
                                    int fontSize)
{
    // задаем шрифт и размер
    label->setFont(QFont("Arial", fontSize, 87));
    // задаем цвет текста
    label->setStyleSheet("color: "+ color);
    // задаем геометрию
    label->setGeometry( this->width()/2 - label->width() + x,
                        this->height()/2 - label->height() + y,
                        label->width()*2,
                        label->height()*2 );
    // выравниваем текст по центру
    label->setAlignment(Qt::AlignCenter);

}

//-----------------------------------------------------------------------------
//  Установка границ скоростей
//-----------------------------------------------------------------------------
double Speedometer::setSpeedRange_(double valSpeed)
 {
    if (valSpeed > valV_max_)
        valSpeed = valV_max_;
    else
    if (valSpeed < 0)
        valSpeed = 0;

    return valSpeed;
}

//-----------------------------------------------------------------------------
//  Парсинг значения скорости в 3х-значное число
//-----------------------------------------------------------------------------
QString Speedometer::parsSpeedCur_(double valSpeed)
{
    QString s1 = ""; // сотни
    QString s2 = ""; // десятки
    QString s3 = ""; // единицы
    int speedRound = 0; //
    int i1 = 0; // сотни
    int i2 = 0; // десятки
    int i3 = 0; // единицы

    speedRound = static_cast<int>(round(valSpeed));
    i1 = speedRound/100;				// Выделяем сотни
    i2 = (speedRound % 100)/10;			// Выделяем десятки
    i3 = speedRound - 100*i1 - 10*i2;	// Выделяем единицы

    // возвращаем строку из 3х-значного числа
    return s1.setNum(i1) + s2.setNum(i2) + s3.setNum(i3) ;
}


//-----------------------------------------------------------------------------
//	Чтение конфигураций спидометра
//-----------------------------------------------------------------------------
bool Speedometer::loadSpeedometerCfg(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString sectionName = "Speedometer";

        // --- основные черточки на шкале спидометра --- //
        // цвет
        if (!cfg.getString(sectionName, "majorLine_color", majorLine_color_))
            majorLine_color_ = "#ffffff";
        // ширина
        if (!cfg.getInt(sectionName, "majorLine_width", majorLine_width_))
            majorLine_width_ = 4;
        // длина
        if (!cfg.getInt(sectionName, "majorLine_length", majorLine_length_))
            majorLine_length_ = 26;



        // макс. скорость
        if (!cfg.getInt(sectionName, "valV_max_OnScale", valV_max_))
            valV_max_ = 300;
        // шаг скорости
        if (!cfg.getInt(sectionName, "valV_step_OnScale", valV_step_))
            valV_step_ = 20;

        // угол центра разрыва дуги
        if (!cfg.getInt(sectionName, "arc_splitPoint", arc_splitPoint_))
            arc_splitPoint_ = 270;
        // длина дуги-разрыва
        if (!cfg.getInt(sectionName, "arc_splitAngle", arc_splitAngle_))
            arc_splitAngle_ = 90;




        // --- текущая скорость на центральном круге --- //
        // пложение
        if (!cfg.getInt(sectionName, "labelValVCenter_x", labelValVCenter_x_))
            labelValVCenter_x_ = 0;
        if (!cfg.getInt(sectionName, "labelValVCenter_y", labelValVCenter_y_))
            labelValVCenter_y_ = 0;
        // цвет
        if (!cfg.getString(sectionName, "labelValVCenter_color", labelValVCenter_color_))
            labelValVCenter_color_ = "#ffffff";
        // размер шрифта
        if (!cfg.getInt(sectionName, "labelValVCenter_fontSize", labelValVCenter_fontSize_))
            labelValVCenter_fontSize_ = 26;

        // --- ограничение скорости под центральным кругом спидометра --- //
        // пложение
        if (!cfg.getInt(sectionName, "labelSpeedLimit_x", labelSpeedLimit_x_))
            labelSpeedLimit_x_ = 0;
        if (!cfg.getInt(sectionName, "labelSpeedLimit_y", labelSpeedLimit_y_))
            labelSpeedLimit_y_ = 70;
        // цвет
        if (!cfg.getString(sectionName, "labelSpeedLimit_color", labelSpeedLimit_color_))
            labelSpeedLimit_color_ = "#ffffff";
        // размер шрифта
        if (!cfg.getInt(sectionName, "labelSpeedLimit_fontSize", labelSpeedLimit_fontSize_))
            labelSpeedLimit_fontSize_ = 26;

    }
    else
    {
        // Параметры по умолчанию.

        // основные черточки на шкале спидометра
        majorLine_color_ = "#ffffff";   // цвет
        majorLine_width_ = 4;           // ширина
        majorLine_length_ = 26;         // длина



        valV_max_ = 300;            // макс. скорость
        valV_step_ = 20;            // шаг скорости

        arc_splitPoint_ = 270;  // угол центра разрыва дуги
        arc_splitAngle_ = 90;   // длина дуги-разрыва




        // текущая скорость на центральном круге
        labelValVCenter_x_ = 0;             // пложение
        labelValVCenter_y_ = 0;             // пложение
        labelValVCenter_color_ = "#ffffff"; // цвет
        labelValVCenter_fontSize_ = 26;     // размер шрифта

        // ограничение скорости под центральным кругом спидометра
        labelSpeedLimit_x_ = 0;             // пложение
        labelSpeedLimit_y_ = 70;            // пложение
        labelSpeedLimit_color_ = "#ffffff"; // цвет
        labelSpeedLimit_fontSize_ = 26;     // размер шрифта

    }

    return true;
}

//-----------------------------------------------------------------------------
//	Чтение конфигураций треугольников ограничений
//-----------------------------------------------------------------------------
bool Speedometer::loadSTriangleCfg(QString cfg_path)
{
    CfgReader cfg;
    QString sectionName = "";

    if (cfg.load(cfg_path))
    {

        // --- параметры ЖЕЛТОГО треугольника ограничения --- //
        sectionName = "STriangle1";
        // ширина
        if (!cfg.getDouble(sectionName, "widthTriangle", widthTriangle_Yellow_))
            widthTriangle_Yellow_ = 6;
        // длина
        if (!cfg.getDouble(sectionName, "lengthTriangle", lengthTriangle_Yellow_))
            lengthTriangle_Yellow_ = 20;
        // цвет
        if (!cfg.getString(sectionName, "colorTriangle", colorTriangle_Yellow_))
            colorTriangle_Yellow_ = "#ffff50";

        // --- параметры КРАСНОГО треугольника ограничения --- //
        sectionName = "STriangle2";
        // ширина
        if (!cfg.getDouble(sectionName, "widthTriangle", widthTriangle_Red_))
            widthTriangle_Red_ = 6;
        // длина
        if (!cfg.getDouble(sectionName, "lengthTriangle", lengthTriangle_Red_))
            lengthTriangle_Red_ = 20;
        // цвет
        if (!cfg.getString(sectionName, "colorTriangle", colorTriangle_Red_))
            colorTriangle_Red_ = "#ff2020";

    }
    else
    {
        // Параметры по умолчанию.

        // параметры ЖЕЛТОГО треугольника ограничения
        widthTriangle_Yellow_ = 6;          // ширина
        lengthTriangle_Yellow_ = 20;        // длина
        colorTriangle_Yellow_ = "#ffff50";  // цвет

        // параметры КРАСНОГО треугольника ограничения
        widthTriangle_Red_ = 6;          // ширина
        lengthTriangle_Red_ = 20;        // длина
        colorTriangle_Red_ = "#ff2020";  // цвет

    }

    return true;
}

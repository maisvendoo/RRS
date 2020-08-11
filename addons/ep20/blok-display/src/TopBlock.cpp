//-----------------------------------------------------------------------------
//
//      Верхний блок БЛОКа
//      (c) РГУПС, ВЖД 30/03/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Верхний блок" БЛОКа
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 30/03/2017
 */


#include "TopBlock.h"
#include "cmath"
#include "CfgReader.h"
#include <QTime>

const	QString		INFORMPARTTRIANGLE_CFG  = "../cfg/BLOK/InformPartTriangle.xml";

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TopBlock::TopBlock(QRect geo, QWidget *parent, QString config_dir) : QWidget(parent)
{
    this->setGeometry(geo);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0,0,0)));
//    this->setStyleSheet("border: 2px solid green");

    // загружаем параметры с конфига
    loadIPTriangleCfg(config_dir + "InformPartTriangle.xml");

//    QTimer* curTime = new QTimer();
//    curTime->setInterval(100);
//    connect(curTime, SIGNAL(timeout()), this, SLOT(onTimer()));
//    // текущее время
//    labelCurTime_ = new QLabel("", this);
//    // нарисовать текущее время
//    drawCurentTime_();
//    curTime->start();

    ip_ = new ip_t();

    QRect geoIP; // размер и положение элементов
    int fooH_ip = 70; // высота элементов
    Qt::Alignment alignLeft = Qt::Alignment(Qt::AlignVCenter + Qt::AlignLeft);
    Qt::Alignment alignCenter = Qt::AlignCenter;

    // устанавливаем элементы информационного блока и заносим в map
    geoIP = QRect(10,0, 200,fooH_ip);
    informPart_ = new InformPart(geoIP, "КООРДИНАТА", "1222км 7пк 19м", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->coordinate, informPart_);
    geoIP = QRect(212,0, 160,fooH_ip);
    informPart_ = new InformPart(geoIP, "СТАНЦИЯ", "ТЕМРЮК", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->station, informPart_);
    geoIP = QRect(374,0, 110,fooH_ip);
    informPart_ = new InformPart(geoIP, "ВРЕМЯ", "14:59:11", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->time, informPart_);
//    geoIP = QRect(530,0, 140,fooH_ip);
//    informPart_ = new InformPart(geoIP, "ПО ГРАФИКУ", "14:59:00", alignCenter, this);
//    informPartMap_.insert(ip_->grafic, informPart_);
    geoIP = QRect(489,0, 50,fooH_ip);
    informPart_ = new InformPart(geoIP, "", "П", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->P, informPart_);
    geoIP = QRect(537,0, 50,fooH_ip);
    informPart_ = new InformPart(geoIP, "", "", alignCenter, this, config_dir);
    informPart_->setImgOverLabel();
    informPartMap_.insert(ip_->P2, informPart_);

    geoIP = QRect(10,80, 80,fooH_ip);
    informPart_ = new InformPart(geoIP, "КАНАЛ", "25Гц", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->chanel, informPart_);
    geoIP = QRect(110,80, 100,fooH_ip);
    informPart_ = new InformPart(geoIP, "№ ПУТИ", "1ПР", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->numRoad, informPart_);
    geoIP = QRect(15,170, 150,fooH_ip);
    informPart_ = new InformPart(geoIP, "УСКОРЕНИЕ", "-0.00", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->acceleration, informPart_);
    geoIP = QRect(15,250, 150,fooH_ip);
    informPart_ = new InformPart(geoIP, "", "1477м", alignCenter, this, config_dir);
    informPart_->setTextOverHead("РАССТ. ДО ЦЕЛИ САУТ", QRect(15,250, 250,fooH_ip));  // РАССТ. ДО ЦЕЛИ САУТ
    informPartMap_.insert(ip_->distanceTargetSAUT, informPart_);
    geoIP = QRect(15,330, 150,fooH_ip);
    informPart_ = new InformPart(geoIP, "КОЭФ. ТОРМ.", "0.47", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->coeffBraking, informPart_);

    geoIP = QRect(10,this->height()-fooH_ip-65, 207,fooH_ip);
    informPart_ = new InformPart(geoIP, "РАССТ. ДО ЦЕЛИ", "", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->distanceTarget, informPart_);
    geoIP = QRect(227,this->height()-fooH_ip-65, 207,fooH_ip);
    informPart_ = new InformPart(geoIP, "ВИД ЦЕЛИ", "", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->typeTarget, informPart_);
    geoIP = QRect(444,this->height()-fooH_ip-65, 207,fooH_ip);
    informPart_ = new InformPart(geoIP, "НАЗВАНИЕ ЦЕЛИ", "", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->nameTarget, informPart_);

    geoIP = QRect(10,this->height()-fooH_ip-10, 640,fooH_ip);
    informPart_ = new InformPart(geoIP, "", "", alignCenter, this, config_dir);
    informPartMap_.insert(ip_->zz, informPart_);

    // ЭЛЕМЕНТЫ ИНФОРМАЦИОННОГО БЛОКА_2 (ДАВЛЕНИЯ В ЦИЛИНДРАХ)
    ipPressureUR_ = new InformPartPressure("УР", this);
    ipPressureTM_ = new InformPartPressure("ТМ", this);
    ipPressureTC_ = new InformPartPressure("ТЦ", this);
    int ipPW = ipPressureUR_->width();
    int ipPY = this->height() - ipPressureUR_->height() - 10;
    ipPressureUR_->move(this->width() - ipPW - 20, ipPY);
    ipPressureTM_->move(ipPressureUR_->x() - ipPW - 10, ipPY);
    ipPressureTC_->move(ipPressureTM_->x() - ipPW - 10, ipPY);


    // размер и положение элементов
    int fooX = this->width() - 40;
    int fooY = 10;
    // устанавливаем КРАСНЫЙ ТРЕУГОЛЬНИК
    fooX -=  widthTriangle_Red_;
    ipTriangleRed_ = new InformPartTriangle( QPoint(fooX, fooY),
                                             widthTriangle_Red_,
                                             QColor(colorTriangle_Red_),
                                             this );

    // устанавливаем ЖЁЛТЫЙ ТРЕУГОЛЬНИК
    fooX -= widthTriangle_Yellow_ + 20;
    ipTriangleYellow_ = new InformPartTriangle( QPoint(fooX, fooY),
                                                widthTriangle_Yellow_,
                                                QColor(colorTriangle_Yellow_),
                                                this );
    ipTriangleYellow_->setTriangle(0, false, true);


    // устанавливаем СПИДОМЕТР
    int speedW = 480;
    speedometer_ = new Speedometer( QRect( this->width()/2 - speedW/2,
                                           -15, // Богос (смещение сверху)
                                           speedW,
                                           speedW ),
                                    this, config_dir );
    speedometer_->move(240, 130);
    speedometer_->setSpeedLimits(120, 80);                                      // zБогос. Удалить.
    speedometer_->setSpeedCur(0.0);                                             // zБогос. Удалить.

    // устанавливаем маленький треугольник возле спидометра снизу справа
    ipTriangle_ = new InformPartTriangle( QPoint(speedometer_->x() - 30,
                                                 speedometer_->y() + speedometer_->height()*0.78),
                                          40,
                                          QColor(255,255,0),
                                          this );
    ipTriangle_->setTriangle(180, true);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TopBlock::~TopBlock()
{

}

//-----------------------------------------------------------------------------
//  Установить значения элементов информационного блока
//-----------------------------------------------------------------------------
void TopBlock::set_ipVal(ip_val_t *val)
{
    informPartMap_[ip_->coordinate]->setText(getCoordinateStr_(val->coordinate));
    informPartMap_[ip_->time]->setText(val->time);
    informPartMap_[ip_->station]->setText(QString(val->station));
    informPartMap_[ip_->numRoad]->setText(QString(val->numRoad));
    informPartMap_[ip_->chanel]->setText(QString(val->chanel));
    informPartMap_[ip_->acceleration]->setText(getAccelerationStr_(val->acceleration));
    informPartMap_[ip_->distanceTargetSAUT]->setText(QString::number(val->distanceTargetSAUT) + "м");
    informPartMap_[ip_->coeffBraking]->setText(QString::number(val->coeffBraking, 'f', 2));
    informPartMap_[ip_->distanceTarget]->setText(QString::number(val->distanceTarget) + "м");
    informPartMap_[ip_->typeTarget]->setText(QString(val->typeTarget));
    informPartMap_[ip_->nameTarget]->setText(QString(val->nameTarget));
    informPartMap_[ip_->zz]->setText(QString(val->zz));
}

//-----------------------------------------------------------------------------
//  Установить значения элементов информационного блока 2
//-----------------------------------------------------------------------------
void TopBlock::set_ip2Val(ip2_val_t *val)
{
    ipPressureUR_->setValPressure(val->UR);
    ipPressureTM_->setValPressure(val->TM);
    ipPressureTC_->setValPressure(val->BC);
}

//-----------------------------------------------------------------------------
// Зажечь "проверка бдительности" (жёлтый треугольник)
//-----------------------------------------------------------------------------
void TopBlock::setTriangleYellow(bool isSignal)
{
    ipTriangleYellow_->setTriangle(0,isSignal,true);
}

//-----------------------------------------------------------------------------
// зажечь ТСКБМ (красный треугольник)
//-----------------------------------------------------------------------------
void TopBlock::setTriangleRed(bool isSignal)
{
    ipTriangleRed_->setTriangle(180, isSignal, false, false);
}

//-----------------------------------------------------------------------------
//  Установить текущее и следующее ограничение скорости
//-----------------------------------------------------------------------------
void TopBlock::setSpeedLimits(int curSpeedLimit, int nextSpeedLimit)
{
    speedometer_->setSpeedLimits(curSpeedLimit, nextSpeedLimit);
}

//-----------------------------------------------------------------------------
//  Установить текущую скорость
//-----------------------------------------------------------------------------
void TopBlock::setCurSpeed(double speed)
{
    speedometer_->setSpeedCur(speed);
}

//-----------------------------------------------------------------------------
//  Вернуть строку координаты, типа: "..км ..пк ..м"
//-----------------------------------------------------------------------------
QString TopBlock::getCoordinateStr_(double coordinate)
{
    if (coordinate <= 0)
    {
        return "0км 0пк 0м";
    }

    int coord_km        = static_cast<int>(coordinate);
    int coord_ostatok   = qRound((coordinate - coord_km) * 1000);
    int coord_pk        = coord_ostatok / 100;// + 1;
    int coord_m         = ((coord_ostatok - 100 * coord_pk) / 10) * 10;

    return  QString("%1км %2пк %3м").arg(coord_km).arg(coord_pk).arg(coord_m);
}

//-----------------------------------------------------------------------------
//  Вернуть строку ускорения, типа: " ' '/'-'<double> "
//-----------------------------------------------------------------------------
QString TopBlock::getAccelerationStr_(double a)
{
    QString fooA_s = "";
    (a >= 0)? (fooA_s = " ") : (fooA_s = "-");
    (qAbs(a) < 0.005)? (fooA_s += "0") : (fooA_s += QString::number(qAbs(a), 'f', 2));

    return fooA_s;
}

//-----------------------------------------------------------------------------
//  Установить текущее время на блоке
//-----------------------------------------------------------------------------
//void TopBlock::onTimer()
//{
//    QTime time = QTime::currentTime();
//    QString text = time.toString("hh:mm:ss");

//    labelCurTime_->setText(text);
//}


//-----------------------------------------------------------------------------
//  Нарисовать текущее время
//-----------------------------------------------------------------------------
//void TopBlock::drawCurentTime_()
//{
//    QString strStyle = "color: "+ QColor(255,255,255).name() + ";" +
//                       "font-weight: bold;";

//    // задаем шрифт и размер
//    labelCurTime_->setFont(QFont("Arial", 24));
//    // задаем цвет текста
//    labelCurTime_->setStyleSheet(strStyle);
//    // задаем геометрию
//    labelCurTime_->resize(200, 45);
//    labelCurTime_->move(0,0);
//    // выравниваем
//    labelCurTime_->setAlignment(Qt::AlignLeft);
//    // сам текст
//    labelCurTime_->setText("00:00:00");

//}


//-----------------------------------------------------------------------------
//  Чтение конфигураций элемента информации (треугольник)
//-----------------------------------------------------------------------------
bool TopBlock::loadIPTriangleCfg(QString cfg_path)
{
    CfgReader cfg;
    QString sectionName = "";

    if (cfg.load(cfg_path))
    {

        // --- параметры ЖЕЛТОГО треугольника --- //
        sectionName = "InformPartTriangle1";
        // ширина
        if (!cfg.getDouble(sectionName, "widthTriangle", widthTriangle_Yellow_))
            widthTriangle_Yellow_ = 120;
        // цвет
        if (!cfg.getString(sectionName, "colorTriangle", colorTriangle_Yellow_))
            colorTriangle_Yellow_ = "#ffff50";

        // --- параметры КРАСНОГО треугольника --- //
        sectionName = "InformPartTriangle2";
        // ширина
        if (!cfg.getDouble(sectionName, "widthTriangle", widthTriangle_Red_))
            widthTriangle_Red_ = 120;
        // цвет
        if (!cfg.getString(sectionName, "colorTriangle", colorTriangle_Red_))
            colorTriangle_Red_ = "#ff2020";

    }
    else
    {
        // Параметры по умолчанию.

        // параметры ЖЕЛТОГО треугольника ограничения
        widthTriangle_Yellow_ = 120;        // ширина
        colorTriangle_Yellow_ = "#ffff50";  // цвет

        // параметры КРАСНОГО треугольника ограничения
        widthTriangle_Red_ = 120;       // ширина
        colorTriangle_Red_ = "#ff2020"; // цвет

    }

    return true;
}


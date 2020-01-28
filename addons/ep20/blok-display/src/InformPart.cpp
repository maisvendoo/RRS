//-----------------------------------------------------------------------------
//
//      Элемент блока информации
//      (c) РГУПС, ВЖД 12/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 12/04/2017
 */

#include "InformPart.h"
#include "CfgReader.h"

const	QString		INFORMPART_CFG = "../cfg/BLOK/InformPart.xml";

//const QString CASSETE_IMG_PREFIX_ = ":/cassete/images/img-cassete/";

//-----------------------------------------------------------------------------
//  Конструктор. Рисуем элементы информационного блока
//-----------------------------------------------------------------------------
InformPart::InformPart(QRect geo, QString strHead, QString strText,
                        Qt::Alignment align,
                        QWidget *parent , QString config_path, int marginLeft, bool isDrawX, bool isDrawO) : QLabel(parent)
{
    // устанавливаем геометрию класса
    this->setGeometry(geo);

    // загружаем параметры с конфига
    loadInformPartCfg(config_path + "InformPart.xml");

    // 1. Текст-заголовок
    QLabel* label_ = new QLabel("", this);
    // задаем шрифт и размер
    label_->setFont(QFont("Arial", fontSize_));
    // задаем цвет заголовка текста
    label_->setStyleSheet( "color: "+ colorTextHead_ + ";" +
                           "font-weight: bold;" );
    // задаем геометрию
    label_->setGeometry( 0,
                         4,
                         this->width(),
                         label_->height() );
    // выравниваем по центру
    label_->setAlignment(Qt::AlignCenter);
    // сам текст
    label_->setText(strHead);


    // 2. Текст внутри прямоугольника
    labelText_ = new QLabel("", this);
    // задаем шрифт и размер
    labelText_->setFont(QFont("Arial", fontSize_));
    // задаем цвет текста
    labelText_->setStyleSheet("color: "+ colorText_ + ";" +
                              "font-weight: bold;" +
                              "border: 2px solid "+ colorBorder_ +";");
    // задаем геометрию
    labelText_->setGeometry( 0,
                             label_->height(),
                             this->width(),
                             this->height() - label_->height() );
    // выравниваем по центру
    labelText_->setAlignment(align);
    labelText_->setContentsMargins(QMargins(marginLeft,0,0,0));
    // сам текст
    labelText_->setText(strText);

    if (isDrawX)
        drawX();
    if (isDrawO)
        drawO();

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
InformPart::~InformPart()
{

}

//-----------------------------------------------------------------------------
//  Установить текст в прямоугольнике элемента информационного блока
//-----------------------------------------------------------------------------
void InformPart::setText(QString strText)
{
    this->labelText_->setText(strText);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InformPart::setTextOverHead(QString headText, QRect geo)
{
    this->setGeometry(geo);

    labelTextOverHead_ = new QLabel("", this);
    // задаем шрифт и размер
    labelTextOverHead_->setFont(QFont("Arial", fontSize_));
    // задаем цвет заголовка текста
    labelTextOverHead_->setStyleSheet( "color: "+ colorTextHead_ + ";" +
                                       "font-weight: bold;");
    // задаем геометрию
    labelTextOverHead_->setGeometry( 0,
                                     4,
                                     250,
                                     labelTextOverHead_->height() );
    // выравниваем по центру
    labelTextOverHead_->setAlignment(Qt::AlignCenter);
    // сам текст
    labelTextOverHead_->setText(headText);

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InformPart::setImgOverLabel()
{
    QPixmap pixmap(":/cassete/img-cassete");
    QLabel* labImg = new QLabel(this);
    labImg->setGeometry(labelText_->geometry());
    labImg->setAlignment(Qt::AlignCenter);
    labImg->setPixmap(pixmap);

}

//-----------------------------------------------------------------------------
//  Нарисовать крестик
//-----------------------------------------------------------------------------
void InformPart::drawX()
{
    // создаем img с размером label
    QImage img = QImage( this->size(),
                         QImage::Format_ARGB32_Premultiplied );


    img.fill(Qt::transparent);

    QPainter paint(&img);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(QPen(QColor(255,0,0),3,Qt::SolidLine));

    int fooMargLeft = 17;
    int fooMargTop = this->height()/2 + 10;
    int fooH = 10;
    int fooW = 8;
    paint.drawLine(fooMargLeft,fooMargTop, fooMargLeft+fooW,fooMargTop+fooH);
    paint.drawLine(fooMargLeft+fooW,fooMargTop, fooMargLeft,fooMargTop+fooH);

    paint.end();

    this->setPixmap(QPixmap::fromImage(img));
}

//-----------------------------------------------------------------------------
//  Нарисовать эллипс
//-----------------------------------------------------------------------------
void InformPart::drawO()
{
    // создаем img с размером label
    QImage img = QImage( this->size(),
                         QImage::Format_ARGB32_Premultiplied );


    img.fill(Qt::transparent);

    QPainter paint(&img);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(QPen(QColor(255,0,0),3,Qt::SolidLine));
    paint.setBrush(QColor(255,0,0));

    paint.drawEllipse(QPoint(this->width()/2, this->height()/4*3-1),
                      this->width()/2-8,
                      this->height()/4-6);

    paint.end();

    this->setPixmap(QPixmap::fromImage(img));
}

//-----------------------------------------------------------------------------
//  Чтение конфигов
//-----------------------------------------------------------------------------
bool InformPart::loadInformPartCfg(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString sectionName = "InformPart";

        // --- параметры спидометра --- //
        // цвет границы
        if (!cfg.getString(sectionName, "colorBorder", colorBorder_))
            colorBorder_ = "#ffff00";
        // цвет текста-заголовка
        if (!cfg.getString(sectionName, "colorTextHead", colorTextHead_))
            colorTextHead_ = "#ffff00";
        // цвет текста
        if (!cfg.getString(sectionName, "colorText", colorText_))
            colorText_ = "#ffffff";
        // размер шрифта
        if (!cfg.getInt(sectionName, "fontSize", fontSize_))
            fontSize_ = 16;

    }
    else
    {
        // Параметры по умолчанию.

        // параметры спидометра
        colorBorder_    = "#ffff00";    // цвет границы
        colorTextHead_  = "#ffff00";    // цвет текста-заголовка
        colorText_      = "#ffffff";    // цвет текста
        fontSize_       = 16;           // размер шрифта

    }

    return true;
}

//-----------------------------------------------------------------------------
//
//      Элемент 2 блока информации
//      (c) РГУПС, ВЖД 13/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 13/04/2017
 */

#include "InformPart2.h"

//-----------------------------------------------------------------------------
//  Конструктор. Рисуем элементы информационного блока
//-----------------------------------------------------------------------------
InformPart2::InformPart2( QRect geo, QString strHead,
                          QWidget *parent ) : QLabel(parent)
{
    this->setGeometry(geo);

    // 1. Текст-заголовок
    setTextLabel_( new QLabel("", this),
                   QRect(0,5,this->width(),this->height()/3),
                   QColor(255,255,0),
                   strHead );

    // 2. Рисуем рамку
    QLabel* label = new QLabel("", this);
    label->setStyleSheet("border: 3px solid "+ QColor(255,255,0).name());
    label->setGeometry( 0,
                        this->height()/3,
                        this->width(),
                        this->height()*2/3 );

    // 3. Рисуем тело элемента
    drawBodyLabel_();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
InformPart2::~InformPart2()
{

}

//-----------------------------------------------------------------------------
//  Установить текст в прямоугольнике элемента информационного блока
//-----------------------------------------------------------------------------
void InformPart2::setVal(double val)
{
    // 1. Устанавливаем численное значение
    QString val_str((qAbs(val) < 0.005)?("0"):(QString::number(val, 'f', 2)));
    this->labelVal_->setText(val_str);

    // 2. Рисуем прогресс бар
    imgBar_.fill(Qt::transparent);

    QPainter paint(&imgBar_);

    paint.setPen(QColor(255, 255, 255));
    paint.setBrush(QColor(255, 255, 255));

    QPolygon rect;
    rect << QPoint(0, 0)
         << QPoint(0, labelBar_->height())
         << QPoint(labelBar_->width()*val, labelBar_->height())
         << QPoint(labelBar_->width()*val, 0);

    paint.drawPolygon(rect);
    paint.end();

    labelBar_->setPixmap(QPixmap::fromImage(imgBar_));
}

//-----------------------------------------------------------------------------
//  Установить label
//-----------------------------------------------------------------------------
void InformPart2::setTextLabel_(QLabel* label, QRect geo, QColor color, QString str)
{
    // задаем шрифт и размер
    label->setFont(QFont("Arial", 14));
    // задаем цвет текста
    label->setStyleSheet( "color: "+ QColor(color).name() + ";" +
                          "font-weight: bold;" );
    // задаем геометрию
    label->setGeometry(geo);
    // выравниваем по центру
    label->setAlignment(Qt::AlignCenter);
    // сам текст
    label->setText(str);
}

//-----------------------------------------------------------------------------
//  Нарисовать тело элемента
//-----------------------------------------------------------------------------
void InformPart2::drawBodyLabel_()
{
    // 1 - прогресс бар
    labelBar_ = new QLabel("", this);
    labelBar_->setStyleSheet("border: 2px solid white;");
    labelBar_->setGeometry( 10,
                            this->height()*2/3 + 4,
                            this->width() - 20,
                            20 + 2 );
    imgBar_ = QImage(labelBar_->size(), QImage::Format_ARGB32_Premultiplied);

    QLabel* label;
    // 1.1 - элемент "|" слева
    label = new QLabel("", this);
    label->setStyleSheet("border: 2px solid white;");
    label->setGeometry( labelBar_->x(),
                        labelBar_->y() - labelBar_->height() +4,
                        2,
                        labelBar_->height() );
    // 1.2 - элемент "0"
    setTextLabel_( new QLabel("", this),
                   QRect(labelBar_->x(), this->height()/2, 20, 25),
                   QColor(255,255,255),
                   "0" );

    // 1.3 - элемент "|" справа
    label = new QLabel("", this);
    label->setStyleSheet("border: 2px solid white;");
    label->setGeometry( labelBar_->x() + labelBar_->width() - 2,
                        labelBar_->y() - labelBar_->height() +4,
                        2,
                        labelBar_->height() );
    // 1.4 - элемент "1"
    setTextLabel_( new QLabel("", this),
                   QRect(labelBar_->x() + labelBar_->width() - 20, this->height()/2, 20, 25),
                   QColor(255,255,255),
                   "1" );

    // 2 - численное значение прогрес-бара
    labelVal_ = new QLabel("", this);
    setTextLabel_( labelVal_,
                   QRect(0, this->height()/3+2, this->width(), this->height()/4),
                   QColor(0,150,255),
                   "0.00" );
}



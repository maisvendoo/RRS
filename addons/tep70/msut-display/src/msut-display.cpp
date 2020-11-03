#include    "msut-display.h"

#include    <QLayout>
#include    <QLabel>
#include    <QPainter>
#include    <QtCore/qmath.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MsutDisplay::MsutDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(644, 465);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0, 0, 0)));
    this->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->setLayout(new QVBoxLayout);
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);




    w_2_ = this->width()/2.0;
    h_2_ = this->height()/2.0;

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MsutDisplay::~MsutDisplay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MsutDisplay::init()
{
    QLabel *background = new QLabel(this);
    background->setFrameShape(QLabel::NoFrame);

    QPixmap pic;

    if (!pic.load(":/msut/msut-main"))
    {
        return;
    }

    background->setFixedWidth(pic.width());
    background->setFixedHeight(pic.height());

    background->setPixmap(pic);


    // ЭТ/Тяга
    scaleArrow_ = new ScaleArrow(QSize(220, 100), 28, background);
    scaleArrow_->setIsArrow(true);
    scaleArrow_->setMaxVal(/*60*/2.0);
    scaleArrow_->move(307, 145);
    //scaleArrow_->setStyleSheet("border: 2px solid red;");
    scaleArrow_->setVal(/*60*/ 1.00 + 1.0);

    labelArrow_ = new QLabel("11", this);
    drawNumberLabel_(labelArrow_, QRect(376,220, 30,20), 12, "red", Qt::AlignRight);
    labelArrow_->setText("120");


    // Скорость
    scaleSpeed_ = new ScaleArrow(QSize(94, 90), 40, background);
    scaleSpeed_->setMaxVal(/*200*/1.0);
    scaleSpeed_->move(312, 40);
    //scaleSpeed_->setStyleSheet("border: 2px solid red;");
    scaleSpeed_->setVal(/*2*/1.0);

    labelSpeed_ = new QLabel("11", this);
    drawNumberLabel_(labelSpeed_, QRect(315,105, 30,15), 10, "black", Qt::AlignRight);
    labelSpeed_->setText("120");


    // Ускорение
    scaleAcceleration_ = new ScaleArrow(QSize(94, 90), 40, background);
    scaleAcceleration_->setMaxVal(/*4*/2);
    scaleAcceleration_->move(427, 40);
    //scaleAcceleration_->setStyleSheet("border: 2px solid red;");
    scaleAcceleration_->setVal(/*2*/ 1.00 + 1.0);

    labelAcceleration_ = new QLabel("11", this);
    drawNumberLabel_(labelAcceleration_, QRect(428,105, 30,15), 10, "black", Qt::AlignRight);
    labelAcceleration_->setText("120");




    //
    //createLab_();
    //drawScaleArrow(background);
    //background->setPixmap(pic);



    this->layout()->addWidget(background);

    AbstractDisplay::init();
}



void MsutDisplay::drawNumberLabel_(QLabel* lab, QRect geo, int fontSize, QString color, Qt::Alignment align)
{
    lab->resize(geo.size());
    lab->move(geo.x(), geo.y());
    //lab->setStyleSheet("border: 1px solid red;");
    lab->setStyleSheet("color:"+ color +";");
    lab->setAlignment(align);
    lab->setFont(QFont("Arial", fontSize, 63));
}



////------------------------------------------------------------------------------
////
////------------------------------------------------------------------------------
//void MsutDisplay::createLab_()
//{
//    label_ = new QLabel(this);
//    label_->resize(200, 150);
//    label_->setStyleSheet("border: 1px solid red;");
//}


////------------------------------------------------------------------------------
////
////------------------------------------------------------------------------------
//void MsutDisplay::drawScaleArrow(QLabel* label)
//{
//    img_.fill(Qt::transparent);
//    QPixmap pix = QPixmap::fromImage(img_);
//    QPainter paint(&pix);
//    paint.setRenderHint(QPainter::Antialiasing, true);

//    int sgp_angleArcEnd = 90;
//    int sgp_maxSpeedScale = 30;
//    int stepDeg_ = 10;
//    double koefLength_ = 0.8;
//    int widthBase_ = 5;



//    double angleInDeg = (360.0-sgp_angleArcEnd) - (sgp_maxSpeedScale - 0)*stepDeg_;

//    double angle = qDegreesToRadians(angleInDeg);
//    double foo   = qDegreesToRadians(90.0);

//    paint.setPen(QPen(Qt::red, 1, Qt::SolidLine));
//    paint.setBrush(Qt::red);

//    QPolygonF triangle;
//    triangle << QPointF( w_2_ + (w_2_*koefLength_)*cos(angle),
//                         h_2_ + (h_2_*koefLength_)*sin(angle) )
//             << QPointF( w_2_ + widthBase_*cos(angle+foo),
//                         h_2_ + widthBase_*sin(angle+foo) )
//             << QPointF( w_2_ + widthBase_*cos(angle-foo),
//                         h_2_ + widthBase_*sin(angle-foo) );

//    paint.drawPolygon(triangle);

//    paint.end();
//    label->setPixmap(pix);
//}



GET_DISPLAY(MsutDisplay);

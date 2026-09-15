#include "manometer.h"

#include    <math.h>
#include    <QFontDatabase>



Manometer::Manometer(QSize _size, QString manName, int maxValScale,
                     QWidget *parent, bool _flag, QString labelColor)
    : QLabel(parent)
{
    Q_UNUSED(_flag)

    this->resize(_size);


    this->setFrameShape(QLabel::NoFrame);
    QPixmap pic;
    if (!pic.load(":/rcc/" + manName)) { return; }
    this->setFixedSize(pic.size());
    this->setPixmap(pic);

    int id = QFontDatabase::addApplicationFont(":/rcc/msud-ttf"); //путь к шрифту
    familyFont_ = QFontDatabase::applicationFontFamilies(id).at(0); //имя шрифта



    manArrows_ = new ManometerArrow(QSize(310, 111), maxValScale, this);
    manArrows_->move(10, 48);

    labColor = labelColor;

    //manArrows_->setVals(0, 0);

//    labVal1_ = new QLabel("0", this);
//    labVal1_->resize(65, 28);
//    labVal1_->setFont(QFont(familyFont_, 20, 0));
//    labVal1_->setStyleSheet("color: yellow;");
//    labVal1_->setMargin(-4);
//    labVal1_->setIndent(6);
//    labVal1_->setAlignment(Qt::AlignRight);



//    createLab_(labVal1_, QSize(65, 28));

//    if (_flag)
//        labVal1_->move(65, 162);
//    else
//        labVal1_->move(80, 162);

//    if (_flag)
//    {
//        createLab_(labVal2_, QSize(65, 28));
//        labVal2_->move(209, 162);
//    }
}



void Manometer::addTxtLab1(QRect geo)
{
    createLab_(labVal1_, geo);
}



void Manometer::addTxtLab2(QRect geo)
{
    createLab_(labVal2_, geo);
}




void Manometer::setVal_Line(int val)
{
    int zz = manArrows_->setVal_Line(val);

    if (zz != -1)
        labVal1_->setText(QString::number(zz));
}



void Manometer::setVal_Arrow(int val)
{
    int zz = manArrows_->setVal_Arrow(val);

    if (zz != -1)
        labVal2_->setText(QString::number(zz));
}



void Manometer::setVal_zonaVIP(double val)
{
    int zz = std::ceil(manArrows_->setVal_LineD(val));

    if (zz >= 0)
        labVal1_->setText(QString::number(zz));
}



void Manometer::createLab_(QLabel *&lab, QRect geo)
{
    lab = new QLabel("0", this);
    lab->setGeometry(geo);
    lab->setFont(QFont(familyFont_, 20, 0));
    lab->setStyleSheet(QString("color: %1;").arg(labColor));//"color: green;");
    lab->setMargin(-4);
    lab->setIndent(6);
    lab->setAlignment(Qt::AlignRight);
}

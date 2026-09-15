#include "block-top.h"

#include "cmath"
/*
#include <QTime>
*/


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TopBlock::TopBlock(QSize size, QWidget *parent)
    : QLabel(parent)
{
    this->resize(size);
    //this->setStyleSheet("border: 1px solid red");


    // Индикация кассеты
    indicationCassette_ = new ImageWidget("rcc", "ind_cassette", QSize(20,20), this);
    indicationCassette_->move(266, 1);
    indicationCassette_->setVisible(true);

    // Индикация М
    indicationM_ = new ImageWidget("rcc", "ind_M", QSize(16,14), this);
    indicationM_->move(198, 7);
    indicationM_->setVisible(true);

    // Индикация П
    indicationP_ = new ImageWidget("rcc", "ind_P", QSize(14,14), this);
    indicationP_->move(234, 7);
    indicationP_->setVisible(true);

    // Индикация из АЛС-ЕН "прямо"
    indicationStraight_ = new ImageWidget("rcc", "ind_straight", QSize(35,14), this);
    indicationStraight_->move(0, 45);
    indicationStraight_->setVisible(true);

    // Индикация из АЛС-ЕН "отклонение"
    indicationSide_ = new ImageWidget("rcc", "ind_side", QSize(35,14), this);
    indicationSide_->move(0, 76);
    indicationSide_->setVisible(true);

    // Индикация "Проверка бдительности"
    indVigilanceCheck_ = new ImageWidget("rcc", "ind_vigilance", QSize(60,63), this);
    indVigilanceCheck_->move(445, 9);
    indVigilanceCheck_->setVisible(true);

    //
    txtPaintCoordinate1_ = new TextPaint(QSize(55, 20), this);
    txtPaintCoordinate1_->move(53, 61);
    txtPaintCoordinate1_->setFonts(16, Qt::green);
    txtPaintCoordinate1_->setParams(4, 14);
    txtPaintCoordinate1_->setText(QString::number(0));

    txtPaintCoordinate2_ = new TextPaint(QSize(50, 20), this);
    txtPaintCoordinate2_->move(113, 61);
    txtPaintCoordinate2_->setFonts(16, Qt::green);
    txtPaintCoordinate2_->setParams(3, 14);
    txtPaintCoordinate2_->setPointForDigit(1, 18);
    txtPaintCoordinate2_->setText(QString::number(0.0, 'f', 3));

    //
    txtPaintStation_ = new TextPaint(QSize(112, 20), this);
    txtPaintStation_->move(181, 61);
    txtPaintStation_->setFonts(16, Qt::yellow);
    txtPaintStation_->setParams(8, 14, true);
    txtPaintStation_->setText("STATION1");

    //
    txtPaintCurTimeH_ = new TextPaint(QSize(28, 20), this);
    txtPaintCurTimeH_->move(312, 61);
    txtPaintCurTimeH_->setFonts(16, Qt::green);
    txtPaintCurTimeH_->setParams(2, 14);
    txtPaintCurTimeH_->setText(QString::number(0));

    txtPaintCurTimeM_ = new TextPaint(QSize(36, 20), this);
    txtPaintCurTimeM_->move(346, 61);
    txtPaintCurTimeM_->setFonts(16, Qt::green);
    txtPaintCurTimeM_->setParams(2, 14);
    txtPaintCurTimeM_->setPointForDigit(1, 18);
    txtPaintCurTimeM_->setText(QString::number(0));

    txtPaintCurTimeS_ = new TextPaint(QSize(36, 20), this);
    txtPaintCurTimeS_->move(388, 61);
    txtPaintCurTimeS_->setFonts(16, Qt::green);
    txtPaintCurTimeS_->setParams(2, 14);
    txtPaintCurTimeS_->setPointForDigit(1, 18);
    txtPaintCurTimeS_->setText(QString::number(0));

    txtPaintSheduleTimeH_ = new TextPaint(QSize(28, 20), this);
    txtPaintSheduleTimeH_->move(312, 0);
    txtPaintSheduleTimeH_->setFonts(16, Qt::green);
    txtPaintSheduleTimeH_->setParams(2, 14);
    txtPaintSheduleTimeH_->setText(QString::number(0));

    txtPaintSheduleTimeM_ = new TextPaint(QSize(36, 20), this);
    txtPaintSheduleTimeM_->move(346, 0);
    txtPaintSheduleTimeM_->setFonts(16, Qt::green);
    txtPaintSheduleTimeM_->setParams(2, 14);
    txtPaintSheduleTimeM_->setPointForDigit(1, 18);
    txtPaintSheduleTimeM_->setText(QString::number(0));

    txtPaintSheduleTimeS_ = new TextPaint(QSize(36, 20), this);
    txtPaintSheduleTimeS_->move(388, 0);
    txtPaintSheduleTimeS_->setFonts(16, Qt::green);
    txtPaintSheduleTimeS_->setParams(2, 14);
    txtPaintSheduleTimeS_->setPointForDigit(1, 18);
    txtPaintSheduleTimeS_->setText(QString::number(0));
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TopBlock::~TopBlock()
{

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setIndM(bool flag)
{
    indicationM_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setIndP(bool flag)
{
    indicationP_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setCassete(bool flag)
{
    indicationCassette_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setIndStraight(bool flag)
{
    indicationStraight_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setIndSide(bool flag)
{
    indicationSide_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setCoordinate(double coordinate)
{
    if ((coordinate < 0.0) || (coordinate > 9999.999))
        return;

    if (std::abs(coordinate - oldCoordinate_) < 0.001)
        return;

    txtPaintCoordinate1_->setText(QString::number(floor(coordinate)));
    txtPaintCoordinate2_->setText(QString::number(coordinate, 'f', 3));

    oldCoordinate_ = coordinate;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setStationName(QString stationName)
{
    if (stationName.compare(oldStation_, Qt::CaseSensitivity::CaseInsensitive) == 0)
        return;

    txtPaintStation_->setText(stationName.toUpper());

    oldStation_ = stationName;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setCurTime(int h, int m, int s)
{
    if (oldCurH_ != h)
    {
        txtPaintCurTimeH_->setText(QString::number(h));
        oldCurH_ = h;
    }

    if (oldCurM_ != m)
    {
        txtPaintCurTimeM_->setText(QString::number(m));
        oldCurM_ = m;
    }

    if (oldCurS_ != s)
    {
        txtPaintCurTimeS_->setText(QString::number(s));
        oldCurS_ = s;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TopBlock::setSheduleTime(int h, int m, int s)
{
    if (oldSheduleH_ != h)
    {
        txtPaintSheduleTimeH_->setText(QString::number(h));
        oldSheduleH_ = h;
    }

    if (oldSheduleM_ != m)
    {
        txtPaintSheduleTimeM_->setText(QString::number(m));
        oldSheduleM_ = m;
    }

    if (oldSheduleS_ != s)
    {
        txtPaintSheduleTimeS_->setText(QString::number(s));
        oldSheduleS_ = s;
    }
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopBlock::setVigilanceCheck(bool flag)
{
    indVigilanceCheck_->setVisible(flag);
}

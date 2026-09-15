#include "block-SAUT.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SAUTBlock::SAUTBlock(QSize _size, QWidget *parent) : QLabel(parent)
{
    this->resize(_size);
    //this->setStyleSheet("border: 1px solid red");

    // Индикация "САУТ включён"
    indicationSAUT_ON_ = new ImageWidget("rcc", "SAUT_on", QSize(14,8), this);
    indicationSAUT_ON_->move(154, 64);
    indicationSAUT_ON_->setVisible(true);

    // Индикация "САУТ выключён"
    indicationSAUT_OFF_ = new ImageWidget("rcc", "SAUT_off", QSize(14,8), this);
    indicationSAUT_OFF_->move(154, 72);
    indicationSAUT_OFF_->setVisible(true);

    // Индикация "Запрет отпуска"
    indicationZapretOtpuska_ = new ImageWidget("rcc", "SAUT_zapret_otpuska", QSize(40,34), this);
    indicationZapretOtpuska_->move(142, 0);
    indicationZapretOtpuska_->setVisible(true);

    // Координата
    txtPaintCoordinate1_ = new TextPaint(QSize(87,24), this);
    txtPaintCoordinate1_->setFonts(18, Qt::green, TextPaint::LED_7SEGMENT);
    txtPaintCoordinate1_->move(3, 6);
    txtPaintCoordinate1_->setParams(6, 22, false);
    txtPaintCoordinate1_->setText(QString::number(8888));

    txtPaintCoordinate2_ = new TextPaint(QSize(29,24), this);
    txtPaintCoordinate2_->setFonts(18, Qt::green, TextPaint::LED_7SEGMENT);
    txtPaintCoordinate2_->move(85, 6);
    txtPaintCoordinate2_->setParams(1, 24);
    txtPaintCoordinate2_->setPointForDigit(1, 22);
    txtPaintCoordinate2_->setText(QString::number(0.8, 'f', 1));

    // Расстояние
    txtPaintDistToTarget_ = new TextPaint(QSize(118,32), this);
    txtPaintDistToTarget_->setFonts(24, Qt::red, TextPaint::LED_7SEGMENT);
    txtPaintDistToTarget_->move(0, 54);
    txtPaintDistToTarget_->setParams(4, 30, false);
    txtPaintDistToTarget_->setText(QString::number(8888));

    // Числовая индикация скорости
    txtCurSpeed_ = new TextPaint(QSize(88,32), this);
    txtCurSpeed_->setFonts(24, Qt::green, TextPaint::LED_7SEGMENT);
    txtCurSpeed_->move(207, 0);
    txtCurSpeed_->setParams(3, 30, false);
    txtCurSpeed_->setText(QString("888"));

    // Числовая индикация ограничения скорости
    txtCurSpeedLimit_ = new TextPaint(QSize(88,32), this);
    txtCurSpeedLimit_->setFonts(24, Qt::red, TextPaint::LED_7SEGMENT);
    txtCurSpeedLimit_->move(207, 54);
    txtCurSpeedLimit_->setParams(3, 30, false);
    txtCurSpeedLimit_->setText(QString("888"));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SAUTBlock::setIndikatorOn(bool flag)
{
    indicationSAUT_ON_->setVisible(flag);
    indicationSAUT_OFF_->setVisible(!flag);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SAUTBlock::setIndZapretOtpuska(bool flag)
{
    indicationZapretOtpuska_->setVisible(flag);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SAUTBlock::setCoordinate(double coordinate)
{
    if ((coordinate < 0.0) || (coordinate > 9999.9))
        return;

    if (std::abs(coordinate - oldCoordinate_) < 0.1)
        return;

    txtPaintCoordinate1_->setText(QString::number(floor(coordinate)));
    txtPaintCoordinate2_->setText(QString::number(coordinate, 'f', 1));

    oldCoordinate_ = coordinate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SAUTBlock::setDistToTarget(int dist)
{
    if (dist == oldDistToTarget_)
        return;

    txtPaintDistToTarget_->setText(QString::number(dist));

    oldDistToTarget_ = dist;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SAUTBlock::setCurSpeed(int curSpeed)
{
    if (oldSpeed_ == curSpeed)
        return;

    txtCurSpeed_->setText(QString::number(curSpeed));

    oldSpeed_ = curSpeed;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SAUTBlock::setCurSpeedLimit(int curSpeedLimit)
{
    if (oldSpeedLimit_ == curSpeedLimit)
        return;

    txtCurSpeedLimit_->setText(QString::number(curSpeedLimit));

    oldSpeedLimit_ = curSpeedLimit;
}

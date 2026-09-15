#include "block-middle.h"



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MiddleBlock::MiddleBlock(QSize _size, QWidget *parent) : QLabel(parent)
{
    this->resize(_size);
    //this->setStyleSheet("border: 1px solid red");

    // Числовая индикация скорости
    txtCurSpeed_ = new TextPaint(QSize(87,35), this);
    txtCurSpeed_->setFonts(30, Qt::green);
    txtCurSpeed_->move(0, 0);
    txtCurSpeed_->setParams(3, 29);
    txtCurSpeed_->setText(QString("000"));

    // Числовая индикация ограничения скорости
    txtCurSpeedLimit_ = new TextPaint(QSize(87,35), this);
    txtCurSpeedLimit_->setFonts(30, Qt::red);
    txtCurSpeedLimit_->move(0, 68);
    txtCurSpeedLimit_->setParams(3, 29);
    txtCurSpeedLimit_->setText(QString("000"));


    // Моргание скорости, если подходим к ограничению скорости
    connect(&timerForBlink, &QTimer::timeout, [&]()
    {
        if ((oldSpeedLimit_ - oldSpeed_ <= 3) || forceBlinking_)
            txtCurSpeed_->setVisible(!txtCurSpeed_->isVisible());
        else
            txtCurSpeed_->setVisible(true);
    });
    timerForBlink.start(500);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MiddleBlock::setCurSpeed(int curSpeed)
{
    if (oldSpeed_ == curSpeed)
        return;

    txtCurSpeed_->setText(QString::number(curSpeed));

    oldSpeed_ = curSpeed;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MiddleBlock::setCurSpeedLimit(int curSpeedLimit)
{
    if (oldSpeedLimit_ == curSpeedLimit)
        return;

    txtCurSpeedLimit_->setText(QString::number(curSpeedLimit));

    oldSpeedLimit_ = curSpeedLimit;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MiddleBlock::setSpeedLimitVisible(bool flag)
{
    txtCurSpeedLimit_->setVisible(flag);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MiddleBlock::blinkingSpeed(bool flag)
{
    forceBlinking_ = flag;
}



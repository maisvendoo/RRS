#include "block-right.h"

#include "cmath"



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RightBlock::RightBlock(QSize size, QWidget *parent)
    : QLabel(parent)
    , txtPaintPressureTM1_(Q_NULLPTR)
    , txtPaintPressureTM2_(Q_NULLPTR)
    , txtPaintPressureUR1_(Q_NULLPTR)
    , txtPaintPressureUR2_(Q_NULLPTR)
    , txtPaintNumTrack_(Q_NULLPTR)
    , txtPaintAcceleration1_(Q_NULLPTR)
    , txtPaintAcceleration2_(Q_NULLPTR)
    , indicationZapretOtpuska_(Q_NULLPTR)
{
    this->resize(size);
    //this->setStyleSheet("border: 1px solid red");


    //
    txtPaintPressureTM1_ = new TextPaint(QSize(14, 20), this);
    txtPaintPressureTM1_->move(0, 0);
    txtPaintPressureTM1_->setFonts(16, Qt::green);
    txtPaintPressureTM1_->setParams(1, 14);
    txtPaintPressureTM1_->setText(QString("0"));

    txtPaintPressureTM2_ = new TextPaint(QSize(36, 20), this);
    txtPaintPressureTM2_->move(20, 0);
    txtPaintPressureTM2_->setFonts(16, Qt::green);
    txtPaintPressureTM2_->setParams(2, 14);
    txtPaintPressureTM2_->setPointForDigit(1, 18);
    txtPaintPressureTM2_->setText(QString("00"));

    //
    txtPaintPressureUR1_ = new TextPaint(QSize(14, 20), this);
    txtPaintPressureUR1_->move(0, 60);
    txtPaintPressureUR1_->setFonts(16, Qt::green);
    txtPaintPressureUR1_->setParams(1, 14);
    txtPaintPressureUR1_->setText(QString("0"));

    txtPaintPressureUR2_ = new TextPaint(QSize(36, 20), this);
    txtPaintPressureUR2_->move(20, 60);
    txtPaintPressureUR2_->setFonts(16, Qt::green);
    txtPaintPressureUR2_->setParams(2, 14);
    txtPaintPressureUR2_->setPointForDigit(1, 18);
    txtPaintPressureUR2_->setText(QString("00"));

    //
    TextPaint *txtPaintALS = new TextPaint(QSize(60, 20), this);
    txtPaintALS->move(83, 60);
    txtPaintALS->setFonts(16, Qt::green);
    txtPaintALS->setParams(2, 14);
    txtPaintALS->setText("25");

    //
    txtPaintNumTrack_ = new TextPaint(QSize(56, 20), this);
    txtPaintNumTrack_->move(0, 122);
    txtPaintNumTrack_->setFonts(16, Qt::green);
    txtPaintNumTrack_->setParams(4, 14, false);
    txtPaintNumTrack_->setText(QString("1ПР"));

    //
    txtPaintAcceleration1_ = new TextPaint(QSize(14, 20), this);
    txtPaintAcceleration1_->move(71, 122);
    txtPaintAcceleration1_->setFonts(16, Qt::green);
    txtPaintAcceleration1_->setParams(1, 14);
    txtPaintAcceleration1_->setText(QString("0"));

    txtPaintAcceleration2_ = new TextPaint(QSize(22, 20), this);
    txtPaintAcceleration2_->move(91, 122);
    txtPaintAcceleration2_->setFonts(16, Qt::green);
    txtPaintAcceleration2_->setParams(1, 14);
    txtPaintAcceleration2_->setPointForDigit(1, 18);
    txtPaintAcceleration2_->setText(QString("0"));


    // Индикация "Запрет отпуска"
    indicationZapretOtpuska_ = new ImageWidget("rcc", "ind_zapret_otpuska", QSize(53,29), this);
    indicationZapretOtpuska_->move(30, 227);
    indicationZapretOtpuska_->setVisible(true);

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RightBlock::setPressureTM(double pressure)
{
    if ((pressure < 0.0) || (pressure > 9.99))
        return;

    if (std::abs(pressure - oldPressureTM_) < 0.01)
        return;

    txtPaintPressureTM1_->setText(QString::number(floor(pressure)));
    txtPaintPressureTM2_->setText(QString::number(pressure, 'f', 2));

    oldPressureTM_ = pressure;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RightBlock::setPressureUR(double pressure)
{
    if ((pressure < 0.0) || (pressure > 9.99))
        return;

    if (std::abs(pressure - oldPressureUR_) < 0.01)
        return;

    txtPaintPressureUR1_->setText(QString::number(floor(pressure)));
    txtPaintPressureUR2_->setText(QString::number(pressure, 'f', 2));

    oldPressureUR_ = pressure;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RightBlock::setNumTrack(QString trackNum)
{
    if (trackNum.compare(oldTrackNum_, Qt::CaseSensitivity::CaseInsensitive) == 0)
        return;

    txtPaintNumTrack_->setText(trackNum.toUpper());

    oldTrackNum_ = trackNum;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RightBlock::setAcceleration(double a)
{
    if ((a < 0.0) || (a > 9.9))
        return;

    if (std::abs(a - oldAcceleration_) < 0.1)
        return;

    txtPaintAcceleration1_->setText(QString::number(floor(a)));
    txtPaintAcceleration2_->setText(QString::number(a, 'f', 1));

    oldAcceleration_ = a;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RightBlock::setIndZapretOtpuska(bool flag)
{
    indicationZapretOtpuska_->setVisible(flag);
}

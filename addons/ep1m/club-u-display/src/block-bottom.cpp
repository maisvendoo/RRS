#include "block-bottom.h"



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BottomBlock::BottomBlock(QSize size, QWidget *parent)
    : QLabel(parent)
{
    this->resize(size);
    //this->setStyleSheet("border: 1px solid red");


    //
    txtPaintDistToTarget_ = new TextPaint(QSize(60, 20), this);
    txtPaintDistToTarget_->move(0, 0);
    txtPaintDistToTarget_->setFonts(16, Qt::green);
    txtPaintDistToTarget_->setParams(4, 15);
    txtPaintDistToTarget_->setText(QString::number(0));
/*
    //
    txtPaintTargetType_ = new TextPaint(QSize(200, 20), this);
    txtPaintTargetType_->move(123, 2);
    txtPaintTargetType_->setFonts(13, Qt::green);
    txtPaintTargetType_->setParams(10, 19, false, false);
    txtPaintTargetType_->setText("СВЕТОФОР");
    //
    txtPaintTargetName_ = new TextPaint(QSize(77, 20), this);
    txtPaintTargetName_->move(this->width()-90, 2);
    txtPaintTargetName_->setFonts(13, Qt::green);
    txtPaintTargetName_->setParams(4, 18, true);
*/
    txtPaintTargetName_ = new TextPaint(QSize(384, 20), this);
    txtPaintTargetName_->move(70, 0);
    txtPaintTargetName_->setFonts(16, Qt::green);
    txtPaintTargetName_->setParams(24, 16, true);
    txtPaintTargetName_->setText(QString("ABCDEFGHIJKLMNOPQRSTUVWX"));
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BottomBlock::setDistToTarget(int dist)
{
    if (dist == oldDistToTarget_)
        return;

    txtPaintDistToTarget_->setText(QString::number(dist));

    oldDistToTarget_ = dist;
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BottomBlock::setTargetName(QString txt)
{
    if (txt.compare(oldTargetName_, Qt::CaseSensitivity::CaseInsensitive) == 0)
        return;

    txtPaintTargetName_->setText(txt.toUpper());

    oldTargetName_ = txt;
}

#ifndef RIGHTBLOCK_H
#define RIGHTBLOCK_H

#include <QLabel>

#include    "text-paint.h"
#include    "image-widget.h"



class RightBlock : public QLabel
{

public:
    RightBlock(QSize size, QWidget* parent = Q_NULLPTR);

    void setPressureTM(double pressure);
    void setPressureUR(double pressure);
    void setNumTrack(QString trackNum);
    void setAcceleration(double a);
    void setIndZapretOtpuska(bool flag);


private:
    TextPaint   *txtPaintPressureTM1_;
    TextPaint   *txtPaintPressureTM2_;
    TextPaint   *txtPaintPressureUR1_;
    TextPaint   *txtPaintPressureUR2_;
    TextPaint   *txtPaintNumTrack_;
    TextPaint   *txtPaintAcceleration1_;
    TextPaint   *txtPaintAcceleration2_;

    ImageWidget *indicationZapretOtpuska_;

    double oldPressureTM_ = -1.0;
    double oldPressureUR_ = -1.0;
    QString oldTrackNum_ = QString("1ПР");
    double oldAcceleration_ = -1.0;


};

#endif // RIGHTBLOCK_H

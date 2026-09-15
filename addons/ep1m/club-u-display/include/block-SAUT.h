#ifndef SUATBLOCK_H
#define SUATBLOCK_H

#include <QLabel>

#include    "text-paint.h"
#include    "image-widget.h"


class SAUTBlock : public QLabel
{
public:
    SAUTBlock(QSize _size, QWidget* parent = Q_NULLPTR);

    void setIndikatorOn(bool flag);
    void setIndZapretOtpuska(bool flag);
    void setCoordinate(double coordinate);
    void setDistToTarget(int dist);
    void setCurSpeed(int curSpeed);
    void setCurSpeedLimit(int curSpeedLimit);


private:
    ImageWidget* indicationSAUT_ON_ = nullptr;
    ImageWidget* indicationSAUT_OFF_ = nullptr;
    ImageWidget* indicationZapretOtpuska_ = nullptr;

    TextPaint*   txtPaintCoordinate1_ = nullptr;
    TextPaint*   txtPaintCoordinate2_ = nullptr;
    TextPaint*   txtPaintDistToTarget_ = nullptr;
    TextPaint*   txtCurSpeed_ = nullptr;
    TextPaint*   txtCurSpeedLimit_ = nullptr;

    double oldCoordinate_ = 0.0;
    int oldDistToTarget_ = 0;
    int oldSpeed_ = -1;
    int oldSpeedLimit_ = -1;
};

#endif // SUATBLOCK_H

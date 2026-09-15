#ifndef MIDDLEBLOCK_H
#define MIDDLEBLOCK_H

#include <QLabel>
#include <QTimer>


#include    "text-paint.h"



class MiddleBlock : public QLabel
{
public:
    MiddleBlock(QSize _size, QWidget* parent = Q_NULLPTR);

    void setCurSpeed(int curSpeed);
    void setCurSpeedLimit(int curSpeedLimit);

    void setSpeedLimitVisible(bool flag);

    void blinkingSpeed(bool flag);


private:
    TextPaint*   txtCurSpeed_ = nullptr;
    TextPaint*   txtCurSpeedLimit_ = nullptr;

    int oldSpeed_ = -1;
    int oldSpeedLimit_ = -1;

    QTimer timerForBlink;
    bool forceBlinking_ = true;


};

#endif // MIDDLEBLOCK_H

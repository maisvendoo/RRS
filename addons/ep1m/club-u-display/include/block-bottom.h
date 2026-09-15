#ifndef BOTTOMBLOCK_H
#define BOTTOMBLOCK_H

#include <QLabel>

#include    "text-paint.h"



class BottomBlock : public QLabel
{

public:
    BottomBlock(QSize size, QWidget* parent = Q_NULLPTR);

    void setDistToTarget(int dist);
    void setTargetName(QString txt);


private:
    TextPaint   *txtPaintDistToTarget_ = nullptr;
//    TextPaint   *txtPaintTargetType_ = nullptr;
    TextPaint   *txtPaintTargetName_ = nullptr;

    int oldDistToTarget_ = 0;
    QString oldTargetName_ = "ABCDEFGHIJKLMNOPQRSTUVWX";

};

#endif // BOTTOMBLOCK_H

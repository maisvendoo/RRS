#ifndef MSUTMAINDISPMOVE_H
#define MSUTMAINDISPMOVE_H

#include    "display.h"

#include "scale-arrow.h"
#include "horizont-bar.h"


#include    <QLabel>

class MsutMainDispMove : public QLabel
{
public:
    MsutMainDispMove(QLabel* parent = Q_NULLPTR);

    void updateData(display_signals_t input_signals);

    void setMyVisible(bool flag = true);


private:
    ScaleArrow* scaleArrow_;
    ScaleArrow* scaleSpeed_;
    ScaleArrow* scaleAcceleration_;
    QLabel* labelArrow_;
    QLabel* labelSpeed_;
    QLabel* labelAcceleration_;


    QFrame* frameVU1_Ited_;
    QFrame* frameVU1_I_;
    QFrame* frameVU1_U_;
    QFrame* frameVU2_U_;
    QFrame* frameVU2_I_;
    QLabel* labelVU1_Ited_;
    QLabel* labelVU1_I_;
    QLabel* labelVU1_U_;
    QLabel* labelVU2_U_;
    QLabel* labelVU2_I_;

    QLabel* label_kW_left_;
    QLabel* label_kW_right_;

    HorizontBar* hBar_;



    void drawNumberLabel_(QLabel* lab, QRect geo, int fontSize,
                          QString color, Qt::Alignment align = Qt::AlignCenter);



};

#endif // MSUTMAINDISPMOVE_H

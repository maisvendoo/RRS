#ifndef     MSUT_DISPLAY_H
#define     MSUT_DISPLAY_H

#include    "display.h"
#include <QLabel>

#include "scale-arrow.h"
#include "horizont-bar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MsutDisplay : public AbstractDisplay
{
public:

    MsutDisplay(QWidget *parent = Q_NULLPTR,
                Qt::WindowFlags f = Qt::WindowFlags());

    ~MsutDisplay();

    void init();

private:
    QLabel* labelCurDate_;
    QLabel* labelCurTime_;

    ScaleArrow* scaleArrow_;
    ScaleArrow* scaleSpeed_;
    ScaleArrow* scaleAcceleration_;
    QLabel* labelArrow_;
    QLabel* labelSpeed_;
    QLabel* labelAcceleration_;


    QImage img_;

    double w_2_;
    double h_2_;

    QLabel* label_;


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


    QLabel* labelReversorFwd_;
    QLabel* labelReversorBwd_;
    QLabel* labelPositin_;
    QLabel* labelRezim_;


    HorizontBar* hBar_;



    void createLab_();
    void drawScaleArrow(QLabel* label);

    void drawNumberLabel_(QLabel* lab, QRect geo, int fontSize,
                          QString color, Qt::Alignment align = Qt::AlignCenter);

    //void drawBarScale_(QLabel);


};

#endif // MSUT_DISPLAY_H

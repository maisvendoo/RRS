#ifndef     MSUT_DISPLAY_H
#define     MSUT_DISPLAY_H

#include    "display.h"
#include <QLabel>

#include "scale-arrow.h"

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
    ScaleArrow* scaleArrow_;
    ScaleArrow* scaleSpeed_;
    ScaleArrow* scaleAcceleration_;


    QImage img_;

    double w_2_;
    double h_2_;

    QLabel* label_;

    QLabel* labelArrow_;
    QLabel* labelSpeed_;
    QLabel* labelAcceleration_;

    void createLab_();
    void drawScaleArrow(QLabel* label);

    void drawNumberLabel_(QLabel* lab, QRect geo, int fontSize,
                          QString color, Qt::Alignment align = Qt::AlignCenter);


};

#endif // MSUT_DISPLAY_H

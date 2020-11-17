#ifndef MSUTMAINDISPPARKING_H
#define MSUTMAINDISPPARKING_H

#include    "display.h"

#include "scale-arrow-2.h"
#include "horizont-bar.h"


#include    <QLabel>

class MsutMainDispParking : public QLabel
{
public:
    MsutMainDispParking(QLabel* parent = Q_NULLPTR);

    void updateData(display_signals_t input_signals);

    void setMyVisible(bool flag = true);



private:
    ScaleArrow2* scaleArrow_P_oil_;
    ScaleArrow2* scaleArrow_T_whater_;
    ScaleArrow2* scaleArrow_T_oil_;
    ScaleArrow2* scaleArrow_I_AB_;
    ScaleArrow2* scaleArrow_P_fuel_;
    ScaleArrow2* scaleArrow_U_chain_;

    QLabel* label_P_oil_;
    QLabel* label_T_whater_;
    QLabel* label_T_oil_;
    QLabel* label_I_AB_;
    QLabel* label_P_fuel_;
    QLabel* label_U_chain_;


    void drawNumberLabel_(QLabel* lab, QRect geo, int fontSize,
                          QString color = "white", Qt::Alignment align = Qt::AlignCenter);





};

#endif // MSUTMAINDISPPARKING_H

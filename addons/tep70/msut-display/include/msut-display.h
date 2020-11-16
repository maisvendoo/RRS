#ifndef     MSUT_DISPLAY_H
#define     MSUT_DISPLAY_H

#include    "display.h"

#include "scale-arrow.h"
#include "horizont-bar.h"


#include    <QLabel>
#include    <QTimer>

#include "msut-main-disp-move.h"
#include "msut-main-disp-parking.h"



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
    QLabel* background_;
    QPixmap pic_;


    QLabel* labelCurDate_;
    QLabel* labelCurTime_;



    MsutMainDispMove* msutMainDispMove_;
    MsutMainDispParking* msutMainDispParking_;



    QLabel* labelReversorFwd_;
    QLabel* labelReversorBwd_;
    QLabel* labelPositin_;
    QLabel* labelRezim_;

    QLabel* label_Positin_Time_;

//    QTimer timerProkachka_;
//    QTimer timerProkrutka_;
//    QTimer timerOstanov_;
    QTimer timerObratniyOtschet_;
    int timerCount_;




    void drawNumberLabel_(QLabel* lab, QRect geo, int fontSize,
                          QString color = "white", Qt::Alignment align = Qt::AlignCenter);



    QTimer update_timer;

private slots:
    void slotUpdateTimer();

    void slotTimerObratniyOtschet_();
//    void slotTimerProkachka_();
//    void slotTimerProkrutka_();
//    void slotTimerOstanov_();

};

#endif // MSUT_DISPLAY_H

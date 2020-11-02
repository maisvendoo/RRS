#ifndef     KLUB_DISPLAY
#define     KLUB_DISPLAY

#include    "display.h"

#include    "led-lamp.h"

#include    <QTimer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KlubDisplay : public AbstractDisplay
{
public:

    KlubDisplay(QWidget *parent = Q_NULLPTR,
                Qt::WindowFlags f = Qt::WindowFlags());

    ~KlubDisplay();

    void init();

private:

    bool alarm_state;

    LEDLamp *alarm;



    QTimer update_timer;

private slots:

    void slotUpdateTimer();
};

#endif

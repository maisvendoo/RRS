#ifndef     BLOK_DISPLAY_H
#define     BLOK_DISPLAY_H

#include    "display.h"

#include    <QTimer>
#include    <QLabel>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BlokDisplay : public AbstractDisplay
{
public:

    BlokDisplay(QWidget *parent = Q_NULLPTR,
                Qt::WindowFlags f = Qt::WindowFlags());

    ~BlokDisplay();

private:

    QTimer  *updateTimer;
    QLabel  *test;



private slots:

    void slotUpdateTimer();
};

#endif // BLOK_DISPLAY_H

#ifndef     BLOK_DISPLAY_H
#define     BLOK_DISPLAY_H

#include    "display.h"

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


};

#endif // BLOK_DISPLAY_H

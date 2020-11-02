#ifndef     MSUT_DISPLAY_H
#define     MSUT_DISPLAY_H

#include    "display.h"

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
};

#endif // MSUT_DISPLAY_H

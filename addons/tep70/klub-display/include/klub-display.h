#ifndef     KLUB_DISPLAY
#define     KLUB_DISPLAY

#include    "display.h"

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



private slots:

};

#endif

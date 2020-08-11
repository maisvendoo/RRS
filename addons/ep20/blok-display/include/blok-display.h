#ifndef     BLOK_DISPLAY_H
#define     BLOK_DISPLAY_H

#include    "display.h"
#include    "blok-funcs.h"
#include    "structures-BLOK.h"

#include    <QTimer>
#include    <QLabel>

#include    "TopBlock.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BlokDisplay : public AbstractDisplay
{
public:

    BlokDisplay(QWidget *parent = Q_NULLPTR,
                Qt::WindowFlags f = Qt::WindowFlags());

    ~BlokDisplay();

    void init();

private:

    QTimer          *updateTimer;

    TopBlock        *topBlock;

    structs_BLOK_t  structsBLOK;

    void initMainWindow();

    void initTopBlock();

private slots:

    void slotUpdateTimer();
};

#endif // BLOK_DISPLAY_H

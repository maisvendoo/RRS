#ifndef CLUB_U_DISPLAY_H
#define CLUB_U_DISPLAY_H

#include    "display.h"

//class ALSN;
class ImageWidget;
class Speedometer;
class ReverseInd;
class TopBlock;
class MiddleBlock;
class RightBlock;
class BottomBlock;
class SAUTBlock;


class ClubUDisplay : public AbstractDisplay
{
public:
    ClubUDisplay(QWidget* parent = Q_NULLPTR,
                  Qt::WindowFlags f = Qt::WindowFlags());

    ~ClubUDisplay();

    void init();

    void update(double t, double dt);

private:

    ImageWidget*    alsnG4_ = nullptr;
    ImageWidget*    alsnG3_ = nullptr;
    ImageWidget*    alsnG2_ = nullptr;
    ImageWidget*    alsnG1_ = nullptr;
    ImageWidget*    alsnY_ = nullptr;
    ImageWidget*    alsnRY_ = nullptr;
    ImageWidget*    alsnR_ = nullptr;
    ImageWidget*    alsnW_ = nullptr;
    Speedometer*    speedometer_ = nullptr;
    ReverseInd*     reverseInd_ = nullptr;
    TopBlock*       topBlock_ = nullptr;
    MiddleBlock*    middleBlock_ = nullptr;
    RightBlock*     rightBlock_ = nullptr;
    BottomBlock*    bottomBlock_ = nullptr;
    SAUTBlock*      SAUTBlock_ = nullptr;

    double upd_interval = 0.15;
    double upd_time = 0.0;
    int  upd_block = 0;

    void initMainWindow();
    void initBlocks_();
};

#endif // CLUB_U_DISPLAY_H

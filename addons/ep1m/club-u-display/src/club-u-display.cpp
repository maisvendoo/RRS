#include "club-u-display.h"

#include    <QVBoxLayout>
#include    <QLabel>
#include    <QDir>

#include    "CfgReader.h"
#include    "club-u-funcs.h"
#include    "ep1m-signals.h"

#include    "speedometer.h"
#include    "reverse-indication.h"
#include    "block-top.h"
#include    "block-middle.h"
#include    "block-right.h"
#include    "block-bottom.h"
#include    "block-SAUT.h"
#include    <core/get_module.h>


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ClubUDisplay::ClubUDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(1024, 1024);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(Qt::black));

    this->setLayout(new QVBoxLayout);
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ClubUDisplay::~ClubUDisplay()
{

}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ClubUDisplay::init()
{
    initMainWindow();
    initBlocks_();

    AbstractDisplay::init();
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ClubUDisplay::initMainWindow()
{
    CfgReader cfg;

    int     sizeWindow_X = 1024;
    int     sizeWindow_Y = 1024;
    bool    hideCursor = false;
    int     timeInterval = 100;

    if (cfg.load(config_dir + getConfigPath("main.xml")))
    {
        QString sectionName = "Main";
        cfg.getInt(sectionName, "sizeWindow_X", sizeWindow_X);
        cfg.getInt(sectionName, "sizeWindow_Y", sizeWindow_Y);
    }

    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(sizeWindow_X, sizeWindow_Y);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(Qt::black));
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ClubUDisplay::initBlocks_()
{
    // путь к конфигам
    QString cfg_path = config_dir + getConfigPath("");

/*
    // Текстура корпуса в фоновый виджет для отладки в display-player
    QLabel* fon = new QLabel(this);
    fon->setFrameShape(QLabel::NoFrame);
    QPixmap pic;
    if (!pic.load(":/rcc/klub_bil_saut_display")) { return; }
    fon->setFixedSize(pic.size());
    //fon->setGeometry(0,0, pic.size().width(), pic.size().height());
    fon->setPixmap(pic);
    fon->move(0, 0);
    //fon->setStyleSheet("border: 2px solid red");
    this->layout()->addWidget(fon);
    //this->setStyleSheet("border: 1px solid red");
*/

    // Локомотивный светофор
    alsnG4_ = new ImageWidget("rcc", "alsn_green", QSize(110, 54), this);
    alsnG4_->move(682, 113);

    alsnG3_ = new ImageWidget("rcc", "alsn_green", QSize(110, 54), this);
    alsnG3_->move(682, 169);

    alsnG2_ = new ImageWidget("rcc", "alsn_green", QSize(110, 54), this);
    alsnG2_->move(793, 113);

    alsnG1_ = new ImageWidget("rcc", "alsn_green", QSize(110, 54), this);
    alsnG1_->move(793, 169);

    alsnY_ = new ImageWidget("rcc", "alsn_yellow", QSize(110, 54), this);
    alsnY_->move(904, 113);

    alsnRY_ = new ImageWidget("rcc", "alsn_yellow_red", QSize(110, 54), this);
    alsnRY_->move(904, 169);

    alsnR_ = new ImageWidget("rcc", "alsn_red", QSize(110, 54), this);
    alsnR_->move(556, 878);

    alsnW_ = new ImageWidget("rcc", "alsn_white", QSize(110, 54), this);
    alsnW_->move(556, 934);

    // Спидометр
    speedometer_ = new Speedometer(QSize(223,236), cfg_path, this);
    speedometer_->move(207, 667);

    // Индикация реверсора
    reverseInd_ = new ReverseInd(QSize(25,10), this);
    reverseInd_->move(417, 779);

    // Верхний блок
    topBlock_ = new TopBlock(QSize(505, 90), this);
    topBlock_->move(23, 129);

    // Центральный блок
    middleBlock_ = new MiddleBlock(QSize(112, 108), this);
    middleBlock_->move(586, 113);

    // Правый блок
    rightBlock_ = new RightBlock(QSize(112, 256), this);
    rightBlock_->move(558, 586);

    // Нижний блок
    bottomBlock_ = new BottomBlock(QSize(454, 20), this);
    bottomBlock_->move(16, 66);

    // Дисплей САУТ
    SAUTBlock_ = new SAUTBlock(QSize(290, 85), this);
    SAUTBlock_->move(631, 285);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ClubUDisplay::update(double t, double dt)
{
    (void) t;

    // Интервал обновления
    upd_time += dt;
    if ((upd_time < upd_interval) || (signal_id < 0))
    {
        need_repaint = false;
        return;
    }

    input_signals.resize(KLUB_U_ACCELERATION + 1, 0.0f);
    need_repaint = true;
    upd_time = 0.0;

    bool cab1 = (signal_id == KLUB_U_CAB1_POWER);
    bool cab2 = (signal_id == KLUB_U_CAB2_POWER);
    if (!(cab1 || cab2) || input_signals[signal_id] == 0.0f)
    {
        alsnG4_->setVisible(false);
        alsnG3_->setVisible(false);
        alsnG2_->setVisible(false);
        alsnG1_->setVisible(false);
        alsnY_->setVisible(false);
        alsnRY_->setVisible(false);
        alsnR_->setVisible(false);
        alsnW_->setVisible(false);
        speedometer_->setVisible(false);
        reverseInd_->setVisible(false);
        topBlock_->setVisible(false);
        middleBlock_->setVisible(false);
        rightBlock_->setVisible(false);
        bottomBlock_->setVisible(false);
        SAUTBlock_->setVisible(false);

        return;
    }
/*
    alsnG4_->setVisible(true);
    alsnG3_->setVisible(true);
    alsnG2_->setVisible(true);
    alsnG1_->setVisible(true);
    alsnY_->setVisible(true);
    alsnRY_->setVisible(true);
    alsnR_->setVisible(true);
    alsnW_->setVisible(true);
*/
    speedometer_->setVisible(true);
    reverseInd_->setVisible(true);
    topBlock_->setVisible(true);
    middleBlock_->setVisible(true);
    rightBlock_->setVisible(true);
    bottomBlock_->setVisible(true);
    SAUTBlock_->setVisible(true);

    int seconds = static_cast<int>(input_signals[SIGNAL_TIME]);
    topBlock_->setCurTime(seconds / 3600, seconds / 60 % 60, seconds % 60);

    // Обновляем блоки экрана по очереди
    ++upd_block;

    // Блок обновлений №1
    if (upd_block == 1)
    {
        float epk_on = cab1 ? 1.0f : 2.0f;
        if (input_signals[KLUB_U_EPK] == epk_on)
        {
            topBlock_->setCassete(static_cast<bool>(input_signals[KLUB_U_CASSETE]));
            topBlock_->setIndM(static_cast<bool>(input_signals[KLUB_U_M]));
            topBlock_->setIndP(static_cast<bool>(input_signals[KLUB_U_P]));
            topBlock_->setIndStraight(static_cast<bool>(input_signals[KLUB_U_STRAIGHT]));
            topBlock_->setIndSide(static_cast<bool>(input_signals[KLUB_U_SIDE]));
            topBlock_->setVigilanceCheck(static_cast<bool>(input_signals[KLUB_U_BDITELNOST]));
        }
        else
        {
            topBlock_->setCassete(false);
            topBlock_->setIndM(false);
            topBlock_->setIndP(false);
            topBlock_->setIndStraight(false);
            topBlock_->setIndSide(false);
            topBlock_->setVigilanceCheck(false);
        }

        seconds = static_cast<int>(input_signals[KLUB_U_SHEDULE_TIME]);
        topBlock_->setSheduleTime(seconds / 3600, seconds / 60 % 60, seconds % 60);

        topBlock_->setCoordinate(static_cast<double>(input_signals[KLUB_U_COORDINATE]));
        SAUTBlock_->setCoordinate(static_cast<double>(input_signals[KLUB_U_COORDINATE]));
        SAUTBlock_->setDistToTarget(static_cast<int>(input_signals[KLUB_U_TARGET_DIST]));
        bottomBlock_->setDistToTarget(static_cast<int>(input_signals[KLUB_U_TARGET_DIST]));
        return;
    }

    // Блок обновлений №2
    if (upd_block == 2)
    {
        float pUR = static_cast<float>(cab1) * input_signals[KLUB_U_PRESSURE_UR1] +
                    static_cast<float>(cab2) * input_signals[KLUB_U_PRESSURE_UR2];
        rightBlock_->setPressureTM(static_cast<double>(input_signals[KLUB_U_PRESSURE_TM]));
        rightBlock_->setPressureUR(static_cast<double>(pUR));
        rightBlock_->setAcceleration(static_cast<double>(input_signals[KLUB_U_ACCELERATION]));
        rightBlock_->setIndZapretOtpuska(static_cast<bool>(input_signals[KLUB_U_ZAPRET_OTPUSKA]));

        SAUTBlock_->setIndZapretOtpuska(static_cast<bool>(input_signals[KLUB_U_ZAPRET_OTPUSKA]));
        return;
    }

    // Блок обновлений №3
    if (upd_block == 3)
    {
        QString text = "";
        for (size_t i = 0; i < 8; ++i)
        {
            int c = static_cast<int>(input_signals[KLUB_U_STATION_SYMB1 + i]);
            text.push_back(((c > 0) && (c < 65536)) ? QChar(c) : QChar(' '));
        }
        topBlock_->setStationName(text);

        text = "";
        for (size_t i = 0; i < 24; ++i)
        {
            int c = static_cast<int>(input_signals[KLUB_U_STRING_SYMB1 + i]);
            text.push_back(((c > 0) && (c < 65536)) ? QChar(c) : QChar(' '));
        }
        bottomBlock_->setTargetName(text);
        return;
    }

    // Блок обновлений №4
    if (upd_block >= 4)
    {
        float epk_on = cab1 ? 1.0f : 2.0f;
        if (input_signals[KLUB_U_EPK] == epk_on)
        {
            alsnG4_->setVisible((input_signals[KLUB_U_ALSN] == 4.0f) && (input_signals[KLUB_U_ALSN_FB] >= 4.0f));
            alsnG3_->setVisible((input_signals[KLUB_U_ALSN] == 4.0f) && (input_signals[KLUB_U_ALSN_FB] >= 3.0f));
            alsnG2_->setVisible((input_signals[KLUB_U_ALSN] == 4.0f) && (input_signals[KLUB_U_ALSN_FB] >= 2.0f));
            alsnG1_->setVisible(input_signals[KLUB_U_ALSN] == 4.0f);
            alsnY_->setVisible(input_signals[KLUB_U_ALSN] == 3.0f);
            alsnRY_->setVisible(input_signals[KLUB_U_ALSN] == 2.0f);
            alsnR_->setVisible(input_signals[KLUB_U_ALSN] == 1.0f);
            alsnW_->setVisible(input_signals[KLUB_U_ALSN] == 0.0f);

            speedometer_->setSpeeds(static_cast<int>(input_signals[KLUB_U_SPEED]),
                                    static_cast<int>(input_signals[KLUB_U_SPEED_LIMIT]),
                                    static_cast<int>(input_signals[KLUB_U_SPEED_LIMIT_2]));
            reverseInd_->setReverse(static_cast<int>(input_signals[KLUB_U_REVERSOR]));

            middleBlock_->setSpeedLimitVisible(true);
            middleBlock_->setCurSpeed(static_cast<int>(input_signals[KLUB_U_SPEED]));
            middleBlock_->setCurSpeedLimit(static_cast<int>(input_signals[KLUB_U_SPEED_LIMIT]));
            middleBlock_->blinkingSpeed(false);

            SAUTBlock_->setIndikatorOn(true);
            SAUTBlock_->setCurSpeed(static_cast<int>(input_signals[KLUB_U_SPEED]));
            SAUTBlock_->setCurSpeedLimit(static_cast<int>(input_signals[KLUB_U_SPEED_LIMIT]));
        }
        else
        {
            alsnG4_->setVisible(false);
            alsnG3_->setVisible(false);
            alsnG2_->setVisible(false);
            alsnG1_->setVisible(false);
            alsnY_->setVisible(false);
            alsnRY_->setVisible(false);
            alsnR_->setVisible(false);
            alsnW_->setVisible(false);

            speedometer_->setSpeeds(static_cast<int>(input_signals[KLUB_U_SPEED]), -5, -5);
            reverseInd_->setReverse(0);

            middleBlock_->setSpeedLimitVisible(false);
            middleBlock_->setCurSpeed(static_cast<int>(input_signals[KLUB_U_SPEED]));
            middleBlock_->setCurSpeedLimit(0);
            middleBlock_->blinkingSpeed(true);

            SAUTBlock_->setIndikatorOn(false);
            SAUTBlock_->setCurSpeed(static_cast<int>(input_signals[KLUB_U_SPEED]));
            SAUTBlock_->setCurSpeedLimit(0);
        }

        // Сбрасываем счётчик
        upd_block = 0;
        return;
    }
}


// Важная штука, чтобы в RRS работало.
GET_MODULE(ClubUDisplay)

#include    "blok-display.h"
#include    "ep20-signals.h"

#include    "CfgReader.h"

#include    <QVBoxLayout>
#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlokDisplay::BlokDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)

{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(1024, 768);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0, 0, 0)));

    this->setLayout(new QVBoxLayout);
    this-> setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BlokDisplay::~BlokDisplay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlokDisplay::init()
{
    initMainWindow();

    initTopBlock();

    AbstractDisplay::init();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlokDisplay::initMainWindow()
{
    CfgReader cfg;

    int     sizeWindow_X = 1024;
    int     sizeWindow_Y = 768;
    bool    hideCursor = true;
    int     timeInterval = 100;

    if (cfg.load(config_dir + getConfigPath("main.xml")))
    {
        QString sectionName = "Main";
        cfg.getInt(sectionName, "sizeWindow_X", sizeWindow_X);
        cfg.getInt(sectionName, "sizeWindow_Y", sizeWindow_Y);
        cfg.getBool(sectionName, "hideCursor", hideCursor);
        cfg.getInt(sectionName, "timeInterval", timeInterval);
    }

    this->setCursor( hideCursor ? Qt::BlankCursor : Qt::ArrowCursor);

    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(sizeWindow_X, sizeWindow_Y);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0, 0, 0)));

    updateTimer = new QTimer;
    connect(updateTimer, &QTimer::timeout, this, &BlokDisplay::slotUpdateTimer);
    updateTimer->setInterval(timeInterval);    
    updateTimer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlokDisplay::initTopBlock()
{
    int margX = 20;
    int margY = 20;

    QRect rect = QRect(margX + 10,
                       margY,
                       this->width() - margX * 2,
                       this->height() - margY * 2);

    topBlock = new TopBlock(rect, this, config_dir + getConfigPath(""));
    this->layout()->addWidget(topBlock);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BlokDisplay::slotUpdateTimer()
{
    structsBLOK.ip2_val.TM = static_cast<double>(input_signals[BLOK_TM_PRESS]);
    structsBLOK.ip2_val.UR = static_cast<double>(input_signals[BLOK_UR_PRESS]);
    structsBLOK.ip2_val.BC = static_cast<double>(input_signals[BLOK_TC_PRESS]);

    topBlock->set_ip2Val(&structsBLOK.ip2_val);

    structsBLOK.ip_val.coordinate = static_cast<double>(input_signals[BLOK_RAILWAY_COORD]);
    structsBLOK.ip_val.acceleration = 0.0;
    strcpy(structsBLOK.ip_val.station, "Ростов Гл.");

    topBlock->set_ipVal(&structsBLOK.ip_val);

    structsBLOK.other_val.curSpeed = qRound(input_signals[BLOK_VELOCITY]);
    structsBLOK.other_val.curSpeedLimit = qRound(input_signals[BLOK_VELOCITY_CURRENT_LIMIT]);
    structsBLOK.other_val.nextSpeedLimit = qRound(input_signals[BLOK_VELOCITY_NEXT_LIMIT]);

    topBlock->setCurSpeed(structsBLOK.other_val.curSpeed);
    topBlock->setSpeedLimits(structsBLOK.other_val.curSpeedLimit, structsBLOK.other_val.nextSpeedLimit);
}

GET_DISPLAY(BlokDisplay)

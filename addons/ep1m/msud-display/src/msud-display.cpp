#include "msud-display.h"

#include    <QVBoxLayout>
#include    <QLabel>
#include    <QFontDatabase>
#include    <core/get_module.h>
//#include    <QTime>

#include    "ep1m-signals.h"

const QString MODE_AUTO_REG("АВТОРЕГ_НИЕ");
const QString MODE_HAND_REG("РУЧНОЕ");

const QString KO_TC("ТЦ");
const QString KO_DB("ДБ");
const QString KO_MK("МК");
const QString KO_DM("ДМ");
const QString KO_NC("НЧ");
const QString KO_OB("ОБ");
const QString KO_KZ("КЗ");
const QString KO_OV("ОВ");
const QString KO_MPK1("МПК1");
const QString KO_MPK2("МПК2");

const QString REVERSOR_FWD("ВПЕРЕД");
const QString REVERSOR_BWD("НАЗАД");

const QString POWER_CIRCUIT_TRACTION("ТЯГА");
const QString POWER_CIRCUIT_RECUPERATION("РЕКУПЕРАЦИЯ");

const QString POWER_CIRCUIT_STATE_1("СОБРАНА");
const QString POWER_CIRCUIT_STATE_0("РАЗОБРАНА");




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MsudDisplay::MsudDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(800, 608);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0, 0, 0)));

    this->setLayout(new QVBoxLayout);
    this-> setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);


   // this->setStyleSheet("border: 4px solid green");


    int id = QFontDatabase::addApplicationFont(":/rcc/msud-ttf"); //путь к шрифту
    familyFont_ = QFontDatabase::applicationFontFamilies(id).at(0); //имя шрифта
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MsudDisplay::~MsudDisplay()
{

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::init()
{
    initDisplay_();

    AbstractDisplay::init();
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::initDisplay_()
{
    // Фоновый виджет
    fon_ = new QLabel(this);
    fon_->setFrameShape(QLabel::NoFrame);
    QPixmap pic;
    if (!pic.load(":/rcc/msud-fon")) { return; }
    fon_->setFixedSize(pic.size());
    //fon->setGeometry(0,0, pic.size().width(), pic.size().height());
    fon_->setPixmap(pic);
    fon_->move(0, 0);
    //fon_->setStyleSheet("border: 1px solid red");
    this->layout()->addWidget(fon_);


    int fooX = 34;
    int fooY = 51;
    int fooW = 233;
    int fooH = 28;
    int fooDeltaX = 252;

    createLab_(labMode_, QSize(fooW, fooH), "yellow");
    labMode_->move(fooX, fooY);



    int foooW = 51;
    int foooX = fooX + fooDeltaX - 2;

    createLab_(labKO_TC_, QSize(foooW, fooH), "red");
    labKO_TC_->move(foooX, fooY);
    labKO_TC_->setText(KO_TC);

    foooX += foooW - 1;
    createLab_(labKO_DB_, QSize(foooW, fooH), "red");
    labKO_DB_->move(foooX, fooY);
    labKO_DB_->setText(KO_DB);

    foooX += foooW - 1;
    createLab_(labKO_MK_, QSize(foooW, fooH), "red");
    labKO_MK_->move(foooX, fooY);
    labKO_MK_->setText(KO_MK);

    foooX += foooW - 1;
    createLab_(labKO_DM_, QSize(foooW, fooH), "red");
    labKO_DM_->move(foooX, fooY);
    labKO_DM_->setText(KO_DM);

    foooX += foooW - 1;
    createLab_(labKO_NC_, QSize(foooW, fooH), "yellow");
    labKO_NC_->move(foooX, fooY);
    labKO_NC_->setText(KO_NC);

    foooX += foooW - 1;
    createLab_(labKO_OB_, QSize(foooW, fooH), "red");
    labKO_OB_->move(foooX, fooY);
    labKO_OB_->setText(KO_OB);

    foooX += foooW - 1;
    createLab_(labKO_KZ_, QSize(foooW, fooH), "red");
    labKO_KZ_->move(foooX, fooY);
    labKO_KZ_->setText(KO_KZ);

    foooX += foooW - 1;
    createLab_(labKO_OV_, QSize(foooW, fooH), "yellow");
    labKO_OV_->move(foooX, fooY);
    labKO_OV_->setText(KO_OV);

    foooX += foooW;
    createLab_(labKO_MPK_, QSize(83, fooH), "yellow");
    labKO_MPK_->move(foooX, fooY);



    fooY  = 125;

    createLab_(labPC1_, QSize(fooW, fooH), "yellow");
    labPC1_->move(fooX, fooY);

    fooX += fooDeltaX;

    createLab_(labPC2_, QSize(fooW, fooH), "yellow");
    labPC2_->move(fooX, fooY);

    fooX += fooDeltaX;

    createLab_(labPC3_, QSize(fooW, fooH), "red");
    labPC3_->move(fooX, fooY);


    fooY = 188;

    // Тяга
    sbTyaga_ = new StatusBar(QSize(252, 37), 100, 1.0, fon_);
    sbTyaga_->move(38, fooY);

    // Ток возбуждения
    sbCurrent_ = new StatusBar(QSize(252, 37), 1000, 1.0, fon_);
    sbCurrent_->move(417, fooY);

    createLab_(labTyaga_, QSize(62, fooH), "red", Qt::AlignRight);
    labTyaga_->move(303, fooY);

    createLab_(labCurrent_, QSize(78, fooH), "red", Qt::AlignRight);
    labCurrent_->move(683, fooY);



    fooY = 394;

    createLab_(labV1_, QSize(65, fooH), "yellow", Qt::AlignRight);
    labV1_->move(110, fooY);

    createLab_(labV2_, QSize(64, fooH), "yellow", Qt::AlignRight);
    labV2_->move(255, fooY);

    createLab_(labI1_, QSize(83, fooH), "yellow", Qt::AlignRight);
    labI1_->move(475, fooY);

    createLab_(labI2_, QSize(82, fooH), "yellow", Qt::AlignRight);
    labI2_->move(625, fooY);


    // Ослабление поля
    fooY = 492;
    createLab_(labFieldWeak1_, QSize(44, fooH + 4), "white");
    labFieldWeak1_->move(124, fooY);
    labFieldWeak1_->setText("1");
    labFieldWeak1_->setMargin(-1);
    createLab_(labFieldWeak2_, QSize(44, fooH + 4), "white");
    labFieldWeak2_->move(176, fooY);
    labFieldWeak2_->setText("2");
    labFieldWeak2_->setMargin(-1);
    createLab_(labFieldWeak3_, QSize(44, fooH + 4), "white");
    labFieldWeak3_->move(227, fooY);
    labFieldWeak3_->setText("3");
    labFieldWeak3_->setMargin(-1);



    fooY = 449;

    // Ток ЭПТ
    sbCurrentEPT_ = new StatusBar(QSize(253, 37), 10, 0.1, fon_);
    sbCurrentEPT_->move(432, fooY);

    createLab_(labCurrentEPT_, QSize(47, fooH), "red", Qt::AlignRight);
    labCurrentEPT_->move(698, fooY);

    fooY = 513;

    // Напряжение ЭПТ
    sbVoltageEPT_ = new StatusBar(QSize(253, 37), 100, 0.1, fon_);
    sbVoltageEPT_->move(432, fooY);

    createLab_(labVoltageEPT_, QSize(65, fooH), "red", Qt::AlignRight);
    labVoltageEPT_->move(698, fooY);




    //
//    manArrV_ = new ManometerArrow(QSize(310, 111), 160, fon_);
//    manArrV_->move(55, 280);

//    manArrI_ = new ManometerArrow(QSize(310, 111), 1600, fon_);
//    manArrI_->move(434, 280);



    QSize manSize(330, 196);

    man11_ = new Manometer(manSize, "man11", 160, fon_);
    man11_->move(45, 232);
    man11_->addTxtLab1(QRect(65,162, 65,28));
    man11_->addTxtLab2(QRect(209,162, 65,28));
    //man11_->setVal_Line(10);
    //man11_->setVal_Arrow(20);
    man11_->setVisible(true);

    man12_ = new Manometer(manSize, "man12", 1600, fon_);
    man12_->move(423, 232);
    man12_->addTxtLab1(QRect(50,162, 85,28));
    man12_->addTxtLab2(QRect(200,162, 85,28));
    //man12_->setVal_Line(500);
    //man12_->setVal_Arrow(600);
    man12_->setVisible(true);

    man21_ = new Manometer(manSize, "man21", 4, fon_, false, "green");
    man21_->move(45, 232);
    man21_->addTxtLab1(QRect(135,162, 55,28));
    //man21_->setVal_zonaVIP(1.2);
    man21_->setVisible(true);

    man22_ = new Manometer(manSize, "man22", 1600, fon_, false, "green");
    man22_->move(423, 232);
    man22_->addTxtLab1(QRect(120,162, 85,28));
    //man22_->setVal_Line(1600);
    man22_->setVisible(true);







    createLab_(labCurTime_, QSize(135, fooH), "yellow", Qt::AlignLeft);
    labCurTime_->move(640, 560);
/*
    connect(&timeTimer_, &QTimer::timeout, [&]()
    {
        labCurTime_->setText(QTime::currentTime().toString("hh.mm.ss"));
    });
    timeTimer_.start(1000);
*/


    createConnectionTimer_(timerTC_, labKO_TC_);
    createConnectionTimer_(timerDB_, labKO_DB_);
    createConnectionTimer_(timerMK_, labKO_MK_);
    createConnectionTimer_(timerDM_, labKO_DM_);
    createConnectionTimer_(timerNC_, labKO_NC_);
    createConnectionTimer_(timerOB_, labKO_OB_);
    createConnectionTimer_(timerKZ_, labKO_KZ_);
    createConnectionTimer_(timerOV_, labKO_OV_);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::createLab_(QLabel* &lab, QSize size, QString color, Qt::Alignment align)
{
    lab = new QLabel("0", fon_);
    lab->resize(size);
    lab->setFont(QFont(familyFont_, 20, 0));
    lab->setStyleSheet(//"border: 1px solid red;"
                        "color: " + color + ";");
    lab->setMargin(-4);
    lab->setIndent(6);
    lab->setAlignment(align);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::createConnectionTimer_(QTimer &timer, QLabel *&lab)
{
    connect(&timer, &QTimer::timeout, [&]()
    {
        lab->setVisible(!lab->isVisible());
    });
    timer.setInterval(500);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::setStateLabKO_(QLabel *&lab, int signalEnum, QTimer &timer)
{
    int sigVal = static_cast<int>(input_signals[signalEnum]);

    if (sigVal == 2)
    {
        if (!timer.isActive())
            timer.start();
    }
    else
    {
        if (timer.isActive())
            timer.stop();

        lab->setVisible(sigVal);
    }

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MsudDisplay::update(double t, double dt)
{
    (void) t;

    // Интервал обновления
    upd_time += dt;
    if ((upd_time < upd_interval) || (signal_id < 0))
    {
        need_repaint = false;
        return;
    }

    input_signals.resize(MSUD_VIP_ZONE + 1, 0.0f);
    need_repaint = true;
    upd_time = 0.0;

    if (input_signals[signal_id] == 0.0f)
    {
        fon_->setVisible(false);
        return;
    }

    fon_->setVisible(true);

    int seconds = static_cast<int>(input_signals[SIGNAL_TIME]);
    QString cur_time = QString("%1:%2:%3")
                           .arg(seconds / 3600, 2, 10, QChar('0'))
                           .arg(seconds / 60 % 60, 2, 10, QChar('0'))
                           .arg(seconds % 60, 2, 10, QChar('0'));
    labCurTime_->setText(cur_time);

    // Обновляем блоки экрана по очереди
    ++upd_block;

    // Блок обновлений №1 и №3
    if ((upd_block == 1) || (upd_block == 3))
    {
        if (static_cast<int>(input_signals[MSUD_MODE]) == 1)
        {
            //if (man11_->isVisible())
            {
                labMode_->setText(MODE_HAND_REG);

                man11_->setVisible(false);
                man12_->setVisible(false);

                man21_->setVisible(true);
                man22_->setVisible(true);
            }

            //
            man21_->setVal_zonaVIP(input_signals[MSUD_VIP_ZONE]);

            //
            man22_->setVal_Line(input_signals[MSUD_CURRENT_ANHCOR1]);
        }
        else if (static_cast<int>(input_signals[MSUD_MODE]) == 2)
        {
            //if (man21_->isVisible())
            {
                labMode_->setText(MODE_AUTO_REG);

                man21_->setVisible(false);
                man22_->setVisible(false);

                man11_->setVisible(true);
                man12_->setVisible(true);
            }

            //
            man11_->setVal_Line(qAbs(input_signals[MSUD_SPEED2]));
            man11_->setVal_Arrow(qAbs(input_signals[MSUD_SPEED1]));

            //
            man12_->setVal_Line(input_signals[MSUD_CURRENT_ANHCOR2]);
            man12_->setVal_Arrow(input_signals[MSUD_CURRENT_ANHCOR1]);
        }
        else
        {
            labMode_->setText("");
        }

        int tyagaVal = sbTyaga_->setVal(static_cast<int>(input_signals[MSUD_TRACTION]));
        if (tyagaVal != -1)
            labTyaga_->setText(QString::number(tyagaVal));

        int currentVal = sbCurrent_->setVal(static_cast<int>(input_signals[MSUD_CURCUIT_VOZB]));
        if (currentVal != -1)
            labCurrent_->setText(QString::number(currentVal));

        double currentEptVal = sbCurrentEPT_->setVal(static_cast<double>(input_signals[MSUD_CURRENT_EPT]));
        if (currentEptVal != -1)
            labCurrentEPT_->setText(QString::number(currentEptVal, 'f', 1));

        double voltageEptVal = sbVoltageEPT_->setVal(static_cast<double>(input_signals[MSUD_VOLTAGE_EPT]));
        if (voltageEptVal != -1)
            labVoltageEPT_->setText(QString::number(voltageEptVal, 'f', 1));

        return;
    }

    // Блок обновлений №2
    if (upd_block == 2)
    {
        setStateLabKO_(labKO_TC_, MSUD_TC, timerTC_);
        setStateLabKO_(labKO_DB_, MSUD_DB, timerDB_);
        setStateLabKO_(labKO_MK_, MSUD_MK, timerMK_);
        setStateLabKO_(labKO_DM_, MSUD_DM, timerDM_);
        setStateLabKO_(labKO_NC_, MSUD_NC, timerNC_);
        setStateLabKO_(labKO_OB_, MSUD_OB, timerOB_);
        setStateLabKO_(labKO_KZ_, MSUD_KZ, timerKZ_);
        setStateLabKO_(labKO_OV_, MSUD_OV, timerOV_);
        return;
    }

    // Блок обновлений №4
    if (upd_block >= 4)
    {
        if (static_cast<int>(input_signals[MSUD_MPK]) == 1)
            labKO_MPK_->setText(KO_MPK1);
        else if (static_cast<int>(input_signals[MSUD_MPK]) == 2)
            labKO_MPK_->setText(KO_MPK2);
        else
            labKO_MPK_->setText("");


        if (static_cast<int>(input_signals[MSUD_REVERSOR]) == 1)
            labPC1_->setText(REVERSOR_FWD);
        else if (static_cast<int>(input_signals[MSUD_REVERSOR]) == 2)
            labPC1_->setText(REVERSOR_BWD);
        else
            labPC1_->setText(REVERSOR_FWD);

        if (static_cast<int>(input_signals[MSUD_TRACTION_TYPE]) == 1)
        {
            labPC2_->setText(POWER_CIRCUIT_TRACTION);
            labPC2_->setStyleSheet("color: yellow;");
        }
        else if (static_cast<int>(input_signals[MSUD_TRACTION_TYPE]) == 2)
        {
            labPC2_->setText(POWER_CIRCUIT_RECUPERATION);
            labPC2_->setStyleSheet("color: yellow;");
        }
        else
            labPC2_->setText("");

        if (static_cast<int>(input_signals[MSUD_TRACTION_STATE]) == 1)
        {
            labPC3_->setText(POWER_CIRCUIT_STATE_1);
            labPC3_->setStyleSheet("color: yellow;");
        }
        else if (static_cast<int>(input_signals[MSUD_TRACTION_STATE]) == 2)
        {
            labPC3_->setText(POWER_CIRCUIT_STATE_0);
            labPC3_->setStyleSheet("color: red;");
        }
        else
            labPC3_->setText("");



        if (static_cast<bool>(input_signals[MSUD_OSLAB_POLE1]))
            labFieldWeak1_->setStyleSheet("color: white; background: red;");
        else
            labFieldWeak1_->setStyleSheet("color: white; background: none;");

        if (static_cast<bool>(input_signals[MSUD_OSLAB_POLE2]))
            labFieldWeak2_->setStyleSheet("color: white; background: red;");
        else
            labFieldWeak2_->setStyleSheet("color: white; background: none;");

        if (static_cast<bool>(input_signals[MSUD_OSLAB_POLE3]))
            labFieldWeak3_->setStyleSheet("color: white; background: red;");
        else
            labFieldWeak3_->setStyleSheet("color: white; background: none;");

        // Сбрасываем счётчик
        upd_block = 0;
        return;
    }
}

// Важная штука, чтобы в RRS работало.
GET_MODULE(MsudDisplay)

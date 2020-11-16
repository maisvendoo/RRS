#include "msut-main-disp-move.h"

#include    <QLabel>


#include    "tep70-signals.h"




MsutMainDispMove::MsutMainDispMove(QLabel *parent)
    : QLabel(parent)
{
    // ЭТ/Тяга
    scaleArrow_ = new ScaleArrow(QSize(220, 100), 28, parent);
    scaleArrow_->setIsArrow(true);
    scaleArrow_->setMaxVal(60);
    scaleArrow_->move(307, 145);
    //scaleArrow_->setStyleSheet("border: 2px solid red;");
    scaleArrow_->setVal(0.00 + 30.0);

    labelArrow_ = new QLabel(parent);
    drawNumberLabel_(labelArrow_, QRect(376,220, 30,20), 12, "#000080", Qt::AlignRight);
    //labelArrow_->setText("120");


    // Скорость
    scaleSpeed_ = new ScaleArrow(QSize(94, 90), 40, parent);
    scaleSpeed_->setMaxVal(200);
    scaleSpeed_->move(312, 40);
    //scaleSpeed_->setStyleSheet("border: 2px solid red;");
    scaleSpeed_->setVal(0.0);

    labelSpeed_ = new QLabel(parent);
    drawNumberLabel_(labelSpeed_, QRect(315,105, 30,15), 10, "black", Qt::AlignRight);
    //labelSpeed_->setText("120");


    // Ускорение
    scaleAcceleration_ = new ScaleArrow(QSize(94, 90), 40, parent);
    scaleAcceleration_->setMaxVal(4);
    scaleAcceleration_->move(427, 40);
    //scaleAcceleration_->setStyleSheet("border: 2px solid red;");
    scaleAcceleration_->setVal(0.0 + 2.0);

    labelAcceleration_ = new QLabel(parent);
    drawNumberLabel_(labelAcceleration_, QRect(428,105, 30,15), 10, "black", Qt::AlignRight);
    //labelAcceleration_->setText("120");


    // ВУ1
    //
    frameVU1_Ited_ = new QFrame(parent);
    frameVU1_Ited_->setAutoFillBackground(true);
    frameVU1_Ited_->setPalette(QPalette(QColor("#000080")));
    frameVU1_Ited_->resize(13, 0);
    frameVU1_Ited_->move(179, 68);

    labelVU1_Ited_ = new QLabel(parent);
    drawNumberLabel_(labelVU1_Ited_, QRect(150,314, 50,20), 14, "#000080");
    labelVU1_Ited_->setText("0");

    //
    frameVU1_I_ = new QFrame(parent);
    frameVU1_I_->setAutoFillBackground(true);
    frameVU1_I_->setPalette(QPalette(QColor("#800080")));
    frameVU1_I_->resize(13, 0);
    frameVU1_I_->move(229, 68);

    labelVU1_I_ = new QLabel(parent);
    drawNumberLabel_(labelVU1_I_, QRect(204,314, 50,20), 14, "#800080");
    labelVU1_I_->setText("0");

    //
    frameVU1_U_ = new QFrame(parent);
    frameVU1_U_->setAutoFillBackground(true);
    frameVU1_U_->setPalette(QPalette(QColor("#000080")));
    frameVU1_U_->resize(13, 0);
    frameVU1_U_->move(280, 68);

    labelVU1_U_ = new QLabel(parent);
    drawNumberLabel_(labelVU1_U_, QRect(258,314, 50,20), 14, "#000080");
    labelVU1_U_->setText("0");

    // ВУ1
    //
    frameVU2_U_ = new QFrame(parent);
    frameVU2_U_->setAutoFillBackground(true);
    frameVU2_U_->setPalette(QPalette(QColor("#000080")));
    frameVU2_U_->resize(13, 0);
    frameVU2_U_->move(540, 68);

    labelVU2_U_ = new QLabel(parent);
    drawNumberLabel_(labelVU2_U_, QRect(525,314, 50,20), 14, "#000080");
    labelVU2_U_->setText("0");

    //
    frameVU2_I_ = new QFrame(parent);
    frameVU2_I_->setAutoFillBackground(true);
    frameVU2_I_->setPalette(QPalette(QColor("#800080")));
    frameVU2_I_->resize(13, 0);
    frameVU2_I_->move(596, 68);

    labelVU2_I_ = new QLabel(parent);
    drawNumberLabel_(labelVU2_I_, QRect(582,314, 50,20), 14, "#800080");
    labelVU2_I_->setText("0");




    // kW
    //
    label_kW_left_ = new QLabel(parent);
    drawNumberLabel_(label_kW_left_, QRect(327,256, 50,20), 14, "#000080", Qt::AlignLeft);
    label_kW_left_->setText("0");

    //
    label_kW_right_ = new QLabel(parent);
    drawNumberLabel_(label_kW_right_, QRect(456,256, 50,20), 14, "#800080", Qt::AlignRight);
    label_kW_right_->setText("0");

    //
    hBar_ = new HorizontBar(QSize(202, 25), parent);
    hBar_->move(316, 284);


}



void MsutMainDispMove::updateData(display_signals_t input_signals)
{
    scaleArrow_->setVal(30.0 + input_signals[MSUT_ET_T]);
    labelArrow_->setText(QString::number(input_signals[MSUT_ET_T]));
    scaleSpeed_->setVal(input_signals[MSUT_SPEED]);
    labelSpeed_->setText(QString::number(input_signals[MSUT_SPEED]));
    scaleAcceleration_->setVal(2.0 + input_signals[MSUT_ACCELLERATION]);
    labelAcceleration_->setText(QString::number(input_signals[MSUT_ACCELLERATION]));


    int fooH = 231;
    int fooW = 13;
    int fooY0 = 68;
    frameVU1_Ited_->resize(fooW, fooH*input_signals[MSUT_VU1_I_TED]/1.5);
    frameVU1_Ited_->move(frameVU1_Ited_->x(), fooY0 + fooH*(1.5 - input_signals[MSUT_VU1_I_TED])/1.5);
    labelVU1_Ited_->setText(QString::number(input_signals[MSUT_VU1_I_TED]));
    frameVU1_I_->resize(fooW, fooH*input_signals[MSUT_VU1_I]/8);
    frameVU1_I_->move(frameVU1_I_->x(), fooY0 + fooH*(8 - input_signals[MSUT_VU1_I])/8);
    labelVU1_I_->setText(QString::number(input_signals[MSUT_VU1_I]));
    frameVU1_U_->resize(fooW, fooH*input_signals[MSUT_VU1_U]);
    frameVU1_U_->move(frameVU1_U_->x(), fooY0 + fooH*(1 - input_signals[MSUT_VU1_U]));
    labelVU1_U_->setText(QString::number(input_signals[MSUT_VU1_U]));
    frameVU2_U_->resize(fooW, fooH*input_signals[MSUT_VU2_U]/4);
    frameVU2_U_->move(frameVU2_U_->x(), fooY0 + fooH*(4 - input_signals[MSUT_VU2_U])/4);
    labelVU2_U_->setText(QString::number(input_signals[MSUT_VU2_U]));
    frameVU2_I_->resize(fooW, fooH*input_signals[MSUT_VU2_I]/250);
    frameVU2_I_->move(frameVU2_I_->x(), fooY0 + fooH*(250 - input_signals[MSUT_VU2_I])/250);
    labelVU2_I_->setText(QString::number(input_signals[MSUT_VU2_I]));

    label_kW_left_->setText(QString::number(input_signals[MSUT_POWER] - input_signals[MSUT_POWER_OTOPLENIE]));
    label_kW_right_->setText(QString::number(input_signals[MSUT_POWER_OTOPLENIE]));
}



void MsutMainDispMove::setMyVisible(bool flag)
{
    scaleArrow_->setVisible(flag);
    scaleSpeed_->setVisible(flag);
    scaleAcceleration_->setVisible(flag);
    labelArrow_->setVisible(flag);
    labelSpeed_->setVisible(flag);
    labelAcceleration_->setVisible(flag);

    frameVU1_Ited_->setVisible(flag);
    frameVU1_I_->setVisible(flag);
    frameVU1_U_->setVisible(flag);
    frameVU2_U_->setVisible(flag);
    frameVU2_I_->setVisible(flag);
    labelVU1_Ited_->setVisible(flag);
    labelVU1_I_->setVisible(flag);
    labelVU1_U_->setVisible(flag);
    labelVU2_U_->setVisible(flag);
    labelVU2_I_->setVisible(flag);

    label_kW_left_->setVisible(flag);
    label_kW_right_->setVisible(flag);

    hBar_->setVisible(flag);
}



void MsutMainDispMove::drawNumberLabel_(QLabel *lab, QRect geo, int fontSize, QString color, Qt::Alignment align)
{
    lab->resize(geo.size());
    lab->move(geo.x(), geo.y());
    //lab->setStyleSheet("border: 1px solid red;");
    lab->setStyleSheet("color:"+ color +";");
    lab->setAlignment(align);
    lab->setFont(QFont("Arial", fontSize, 63));
}

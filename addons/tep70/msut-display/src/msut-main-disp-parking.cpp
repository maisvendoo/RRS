#include "msut-main-disp-parking.h"

#include    <QLabel>


#include    "tep70bs-signals.h"




MsutMainDispParking::MsutMainDispParking(QLabel *parent)
    : QLabel(parent)
{
    // Давление масла
    scaleArrow_P_oil_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_P_oil_->move(150, 57);
    scaleArrow_P_oil_->setMaxVal(16);
    //scaleArrow_P_oil_->setStyleSheet("border: 2px solid red;");
    scaleArrow_P_oil_->setVal(0);

    // Температура воды
    scaleArrow_T_whater_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_T_whater_->move(318, 57);
    scaleArrow_T_whater_->setMaxVal(120);
    scaleArrow_T_whater_->setVal(0);

    // Температура масла
    scaleArrow_T_oil_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_T_oil_->move(486, 57);
    scaleArrow_T_oil_->setMaxVal(120);
    scaleArrow_T_oil_->setVal(0);

    // Ток заряда АБ
    scaleArrow_I_AB_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_I_AB_->move(150, 220);
    scaleArrow_I_AB_->setMaxVal(150);
    scaleArrow_I_AB_->setVal(0);

    // Давление топлива
    scaleArrow_P_fuel_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_P_fuel_->move(317, 220);
    scaleArrow_P_fuel_->setMaxVal(6);
    scaleArrow_P_fuel_->setVal(0);

    // Напряжение борт. сети
    scaleArrow_U_chain_ = new ScaleArrow2(QSize(150, 130), 0, parent);
    scaleArrow_U_chain_->move(485, 220);
    scaleArrow_U_chain_->setMaxVal(150);
    scaleArrow_U_chain_->setVal(0);




    label_P_oil_ = new QLabel(parent);
    drawNumberLabel_(label_P_oil_, QRect(208,165, 40,20), 12);
    label_P_oil_->setText("0.0");

    label_T_whater_ = new QLabel(parent);
    drawNumberLabel_(label_T_whater_, QRect(376,165, 40,20), 12);
    label_T_whater_->setText("0");

    label_T_oil_ = new QLabel(parent);
    drawNumberLabel_(label_T_oil_, QRect(544,165, 40,20), 12);
    label_T_oil_->setText("0");


    label_I_AB_ = new QLabel(parent);
    drawNumberLabel_(label_I_AB_, QRect(208,329, 40,20), 12);
    label_I_AB_->setText("0");

    label_P_fuel_ = new QLabel(parent);
    drawNumberLabel_(label_P_fuel_, QRect(376,327, 40,20), 12);
    label_P_fuel_->setText("0.0");

    label_U_chain_ = new QLabel(parent);
    drawNumberLabel_(label_U_chain_, QRect(544,327, 40,20), 12);
    label_U_chain_->setText("0");
}



void MsutMainDispParking::updateData(display_signals_t input_signals)
{
    scaleArrow_P_oil_->setVal(input_signals[MSUT_P_OIL]);
    scaleArrow_T_whater_->setVal(input_signals[MSUT_T_WHATER]);
    scaleArrow_T_oil_->setVal(input_signals[MSUT_T_OIL]);
    scaleArrow_I_AB_->setVal(input_signals[MSUT_I_AB]);
    scaleArrow_P_fuel_->setVal(input_signals[MSUT_P_FUEL]);
    scaleArrow_U_chain_->setVal(input_signals[MSUT_U_CHAIN]);

    label_P_oil_->setText(QString::number(input_signals[MSUT_P_OIL], 'f', 1));
    label_T_whater_->setText(QString::number(qRound(input_signals[MSUT_T_WHATER])));
    label_T_oil_->setText(QString::number(qRound(input_signals[MSUT_T_OIL])));
    label_I_AB_->setText(QString::number(input_signals[MSUT_I_AB], 'f', 1));
    label_P_fuel_->setText(QString::number(input_signals[MSUT_P_FUEL], 'f', 1));
    label_U_chain_->setText(QString::number(qRound(input_signals[MSUT_U_CHAIN])));
}



void MsutMainDispParking::setMyVisible(bool flag)
{
    scaleArrow_P_oil_->setVisible(flag);
    scaleArrow_T_whater_->setVisible(flag);
    scaleArrow_T_oil_->setVisible(flag);
    scaleArrow_I_AB_->setVisible(flag);
    scaleArrow_P_fuel_->setVisible(flag);
    scaleArrow_U_chain_->setVisible(flag);

    label_P_oil_->setVisible(flag);
    label_T_whater_->setVisible(flag);
    label_T_oil_->setVisible(flag);
    label_I_AB_->setVisible(flag);
    label_P_fuel_->setVisible(flag);
    label_U_chain_->setVisible(flag);
}


void MsutMainDispParking::drawNumberLabel_(QLabel *lab, QRect geo, int fontSize, QString color, Qt::Alignment align)
{
    lab->resize(geo.size());
    lab->move(geo.x(), geo.y());
    lab->setStyleSheet("color:"+ color +";");
    lab->setAlignment(align);
    lab->setFont(QFont("Arial", fontSize, 63));
}

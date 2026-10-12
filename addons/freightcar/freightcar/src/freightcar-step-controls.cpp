#include    "freightcar.h"

#include    <freightcar-controls.h>
#include    <coupling-operating-rod.h>
#include    <pneumo-anglecock.h>
#include    <pneumo-hose-epb.h>
#include    <Journal.h>

#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::stepControls(const double &t, const double &dt)
{
    (void) t;
    (void) dt;

    // У вагона нет кабин, поэтому все органы управления общие
    auto &shared_inputs = control_inputs[control_inputs.size() - 1];

    // Сигнальные диски "Хвост грузового поезда"
    shared_inputs[CTRL_DISK_END_OF_TRAIN_FWD].toBool() ? disk_end_of_train_fwd.set() : disk_end_of_train_fwd.reset();
    shared_inputs[CTRL_DISK_END_OF_TRAIN_BWD].toBool() ? disk_end_of_train_bwd.set() : disk_end_of_train_bwd.reset();

    // Расцепные рычаги сцепных устройств
    // Передняя сцепка
    if (shared_inputs[CTRL_OPER_ROD_FIX_FWD].toBool())
        oper_rod_fwd->setExternalState(-1.0);
    else
        oper_rod_fwd->setExternalState(shared_inputs[CTRL_OPER_ROD_FWD].value);

    // Задняя сцепка
    if (shared_inputs[CTRL_OPER_ROD_FIX_BWD].toBool())
        oper_rod_bwd->setExternalState(-1.0);
    else
        oper_rod_bwd->setExternalState(shared_inputs[CTRL_OPER_ROD_BWD].value);

    // Концевые краны тормозной магистрали
    shared_inputs[CTRL_ANGLECOCK_BP_FWD].toBool() ? anglecock_bp_fwd->open() : anglecock_bp_fwd->close();
    shared_inputs[CTRL_ANGLECOCK_BP_BWD].toBool() ? anglecock_bp_bwd->open() : anglecock_bp_bwd->close();

    // Рукава тормозной магистрали
    hose_bp_fwd->setExternalState(static_cast<int>(std::round(shared_inputs[CTRL_HOSE_BP_FWD].value)));
    hose_bp_bwd->setExternalState(static_cast<int>(std::round(shared_inputs[CTRL_HOSE_BP_BWD].value)));

    // Тормозные башмаки
    shared_inputs[CTRL_BRAKE_SHOES].toBool() ? brake_shoes_set.set() : brake_shoes_set.reset();
}
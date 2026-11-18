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
    const size_t shared_cab = vc_inputs.size() - 1;

    // Сигнальные диски "Хвост грузового поезда"
    vc_inputs.getBool(shared_cab, CTRL_DISK_END_OF_TRAIN_FWD) ? disk_end_of_train_fwd.set() : disk_end_of_train_fwd.reset();
    vc_inputs.getBool(shared_cab, CTRL_DISK_END_OF_TRAIN_BWD) ? disk_end_of_train_bwd.set() : disk_end_of_train_bwd.reset();

    // Расцепные рычаги сцепных устройств
    // Передняя сцепка
    if (vc_inputs.getBool(shared_cab, CTRL_OPER_ROD_FIX_FWD))
        oper_rod_fwd->setExternalState(-1.0);
    else
        oper_rod_fwd->setExternalState(vc_inputs.getValue(shared_cab, CTRL_OPER_ROD_FWD));

    // Задняя сцепка
    if (vc_inputs.getBool(shared_cab, CTRL_OPER_ROD_FIX_BWD))
        oper_rod_bwd->setExternalState(-1.0);
    else
        oper_rod_bwd->setExternalState(vc_inputs.getValue(shared_cab, CTRL_OPER_ROD_BWD));

    // Концевые краны тормозной магистрали
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BP_FWD) ? anglecock_bp_fwd->open() : anglecock_bp_fwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BP_BWD) ? anglecock_bp_bwd->open() : anglecock_bp_bwd->close();

    // Рукава тормозной магистрали
    hose_bp_fwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BP_FWD))));
    hose_bp_bwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BP_BWD))));

    // Тормозные башмаки
    vc_inputs.getBool(shared_cab, CTRL_BRAKE_SHOES) ? brake_shoes_set.set() : brake_shoes_set.reset();
}
#include    "freightcar.h"
#include    "freightcar-signals.h"
#include    "pneumo-anglecock.h"
#include    "pneumo-hose-epb.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::signalsOutput(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    // Углы поворота колёсных пар для анимаций
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);

    // Наличие тормозных колодок
    analogSignal[IS_BRAKE_SHOWS] = static_cast<float>(brake_shoes_set.getState());

    // Сигнальные диски "Хвост грузового поезда"
    analogSignal[DISK_END_OF_TRAIN_FWD] = static_cast<float>(disk_end_of_train_fwd.getState());
    analogSignal[DISK_END_OF_TRAIN_BWD] = static_cast<float>(disk_end_of_train_bwd.getState());

    // Углы переднего рукава тормозной магистрали
    // Рукав закреплён на концевом кране с отростком вниз под 45 градусов,
    // и концевой крепится на трубу ТМ с поворотом на 60 градусов внутрь,
    // из суммы этих вращений, у рукава изначально уже есть поворот на:
    // 40.894 градуса вправо, вокруг вертикальной оси
    // 20.704 градуса вниз, вокруг поперечной оси
    analogSignal[HOSE_BP_FWD_ANGLE_SIDE] = hose_bp_fwd->getSideAngle() + 1.0;
    analogSignal[HOSE_BP_FWD_ANGLE_DOWN] = hose_bp_fwd->getDownAngle() + 1.0;
    analogSignal[HOSE_BP_FWD_ANGLE_HEAD_UP] = hose_bp_fwd->getHeadUpAngle() + 1.0;

    analogSignal[HOSE_BP_BWD_ANGLE_SIDE] = hose_bp_bwd->getSideAngle() + 1.0;
    analogSignal[HOSE_BP_BWD_ANGLE_DOWN] = hose_bp_bwd->getDownAngle() + 1.0;
    analogSignal[HOSE_BP_BWD_ANGLE_HEAD_UP] = hose_bp_bwd->getHeadUpAngle() + 1.0;

    // Рукоятка переднего концевого крана тормозной магистрали
    analogSignal[ANGLECOCK_BP_FWD_STATE] = anglecock_bp_fwd->getHandlePosition();

    // Рукоятка заднего концевого крана тормозной магистрали
    analogSignal[ANGLECOCK_BP_BWD_STATE] = anglecock_bp_bwd->getHandlePosition();
}

#include    "passcar.h"
#include    "passcar-signals.h"
#include    "pneumo-anglecock.h"
#include    "pneumo-hose-epb.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::signalsOutput(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    // Углы поворота колёсных пар для анимаций
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
    {
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);
    }

    // Наличие тормозных колодок
    analogSignal[IS_BRAKE_SHOWS] = static_cast<float>(brake_shoes_set.getState());

    // Три красных сигнальных огня хвостового вагона
    analogSignal[RED_LAMPS_END_OF_TRAIN_FWD] = static_cast<float>(red_lamps_end_of_train_fwd.getState());
    analogSignal[RED_LAMPS_END_OF_TRAIN_BWD] = static_cast<float>(red_lamps_end_of_train_bwd.getState());

    // Углы переднего рукава тормозной магистрали
    analogSignal[HOSE_BP_FWD_ANGLE_SIDE] = hose_bp_fwd->getSideAngle() + 1.0;
    analogSignal[HOSE_BP_FWD_ANGLE_DOWN] = hose_bp_fwd->getDownAngle() + 1.0;
    analogSignal[HOSE_BP_FWD_ANGLE_HEAD_UP] = hose_bp_fwd->getHeadUpAngle() + 1.0;
    // Рукоятка переднего концевого крана тормозной магистрали
    analogSignal[ANGLECOCK_BP_FWD_STATE] = anglecock_bp_fwd->getHandlePosition();

    // Углы заднего рукава тормозной магистрали
    analogSignal[HOSE_BP_BWD_ANGLE_SIDE] = hose_bp_bwd->getSideAngle() + 1.0;
    analogSignal[HOSE_BP_BWD_ANGLE_DOWN] = hose_bp_bwd->getDownAngle() + 1.0;
    analogSignal[HOSE_BP_BWD_ANGLE_HEAD_UP] = hose_bp_bwd->getHeadUpAngle() + 1.0;
    // Рукоятка заднего концевого крана тормозной магистрали
    analogSignal[ANGLECOCK_BP_BWD_STATE] = anglecock_bp_bwd->getHandlePosition();

    // Вращение оси подвагонного генератора
    analogSignal[GENERATOR_AXIS] = static_cast<float>(wheel_rotation_angle[2] * ip / 2.0 / Physics::PI);

    // Освещение в вагоне
    analogSignal[INTERIOR_LIGHT] = static_cast<float>(interior_light.getState());
}

#include    "passcar.h"
#include    "passcar-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::signalsOutput(const simulator_time_t& t, const double& dt)
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
    analogSignal[RED_LAMPS_END_OF_TRAIN_FWD] = static_cast<float>(red_lamps_end_of_train_fwd.getState());
    analogSignal[RED_LAMPS_END_OF_TRAIN_BWD] = static_cast<float>(red_lamps_end_of_train_bwd.getState());

    // Вращение оси подвагонного генератора
    analogSignal[GENERATOR_AXIS] = static_cast<float>(wheel_rotation_angle[2] * ip / 2.0 / Physics::PI);
}

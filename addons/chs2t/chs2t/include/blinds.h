//------------------------------------------------------------------------------
//
//      Жалюзи пуско-тормозных резисторов (ПТР)
//
//------------------------------------------------------------------------------
#ifndef     BLINDS_H
#define     BLINDS_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Blinds : public Device
{
public:

    Blinds(QObject *parent = Q_NULLPTR);

    ~Blinds();

    /// Задать состояние жалюзи (открыто/закрыто)
    void setState(bool state) { this->state = state; }

    /// Признак полностью открытых жалюзи
    bool isOpened() const { return is_opened; }

    /// Получить текущее положение жалюзи
    float getPosition() const { return static_cast<float>(getY(0)); }

private:

    /// Время перемещения между открытым и закрытым состояниями
    double  motion_time;

    /// Заданное состояние жалюзи
    bool    state;

    /// Признак открытия
    bool    is_opened;

    /// Максимальная величина положения жалюзи (в относительном выражении)
    double  max_pos;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // BLINDS_H

#ifndef     PANTOGRAPH_H
#define     PANTOGRAPH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Pantograph : public Device
{
public:

    Pantograph(QString config_path, QObject *parent = Q_NULLPTR);

    ~Pantograph();

    void setState(bool state);

    void setUks(double Uks);

    double getHeight() const;

    double getUout() const;

private:

    /// Напряжение в контактной сети
    double  Uks;

    /// Напряжение на выходе токоприемника
    double  Uout;

    /// Состояние токоприемника (true - подъем, false - опускание)
    bool    state;

    /// Максимальная высота подъема (в относительном выражении)
    double  max_height;

    /// Время подъема/опускания ТП
    double  motion_time;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void load_config(QString cfg_path);
};

#endif // PANTOGRAPH_H

#ifndef     COUPLING_SA3_H
#define     COUPLING_SA3_H

#include    "coupling.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CouplingSA3 : public Coupling
{
public:

    CouplingSA3(QObject *parent = Q_NULLPTR);

    ~CouplingSA3();

    /// Переопределение управления сцепкой
    virtual void setCouplingOperatingState(double state);

    /// Переопределение сцепления, для отмены срабатывания замкодержателя
    virtual void couple();

    /// Переопределение расцепления, для срабатывания замкодержателя
    virtual void uncouple();

    virtual void step(double t, double dt);

private:

    /// Признак сработавшего замкодержателя (замок расцеплен до разведения автосцепок)
    bool is_lockkeeper_working;

    void load_config(CfgReader &cfg);
};

#endif // COUPLING_SA3_H

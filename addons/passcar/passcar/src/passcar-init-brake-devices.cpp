#include    "passcar.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::initBrakeDevices(double p0, double pBP, double pFL)
{
    Q_UNUSED(p0)

    // Инициализация давления в тормозной магистрали
    brakepipe->setY(0, pBP);

    // Инициализация давления в воздухораспределителе
    if (air_dist != nullptr)
        air_dist->init(pBP, pFL);

    // Инициализация давления в электровоздухораспределителе
    if (electro_air_dist != nullptr)
        electro_air_dist->init(pBP, pFL);

    // Инициализация давления в запасном резервуаре
    supply_reservoir->setY(0, pBP);

    // Инициализация давления в концевых кранах тормозной магистрали
    anglecock_bp_fwd->setPipePressure(pBP);
    anglecock_bp_bwd->setPipePressure(pBP);

    // Инициализация давления в рукавах тормозной магистрали
    hose_bp_fwd->setPressure(pBP);
    hose_bp_bwd->setPressure(pBP);

    // Состояние рукавов и концевых кранов тормозной магистрали
    if (hose_bp_fwd->isLinked())
    {
        hose_bp_fwd->connect();
        anglecock_bp_fwd->open();
    }
    else
    {
        anglecock_bp_fwd->close();
    }

    if (hose_bp_bwd->isLinked())
    {
        hose_bp_bwd->connect();
        anglecock_bp_bwd->open();
    }
    else
    {
        anglecock_bp_bwd->close();
    }
}

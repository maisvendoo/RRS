#include    "passcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::initBrakepipe(QString module_path)
{
    Q_UNUSED(module_path)

    // Тормозная магистраль
    double volume = length * 0.0343 * 0.0343 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volume);
    //DebugMsg = QString("%1").arg(volume, 9, 'f', 6);

    // Рукава тормозной магистрали
    volume = 0.7 * 0.0343 * 0.0343 * Physics::PI / 4.0;
    hose_tm_fwd = new Reservoir(volume);
    hose_tm_bwd = new Reservoir(volume);
    //DebugMsg += QString("%1").arg(volume, 9, 'f', 6);

    // Концевые краны
    anglecock_tm_fwd = new PneumoAngleCock();
    anglecock_tm_fwd->read_config("pneumo-anglecock");
    anglecock_tm_bwd = new PneumoAngleCock();
    anglecock_tm_bwd->read_config("pneumo-anglecock");
}

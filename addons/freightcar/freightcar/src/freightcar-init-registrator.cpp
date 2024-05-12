#include    "freightcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::initRegistrator()
{
    registrator = new Registrator();
    registrator->setFileName(QString("freightcar%1m_%2").arg(length,4,'f',1,'0').arg(idx));
    registrator->read_custom_config(config_dir + QDir::separator() + "registrator");
    registrator->init();

    if (air_dist_module == "vr483")
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pRK   ; pZK   ; pKDR  ; pBCref; BPsr   ; MKzk km; MKzk pl; ZKrk gp; ZKrk pd; MKrk pd; ZKkdr  ; ZKkdr d; MKkdr d; KDRbc  ; KDRatm ; KDRatmd; SRbc f ; SRbc s ; BCatm  ; poz d ; poz gp; poz up"));
}

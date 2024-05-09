#include    "freightcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::initRegistrator()
{
    registrator = new Registrator();
    registrator->setFileName(QString("freightcar_%1_%2").arg(air_dist_module).arg(idx));
    registrator->read_custom_config(config_dir + QDir::separator() + "registrator");
    registrator->init();

    if (air_dist_module == "vr292")
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pKDR  ; BPsr   ; BPkdr  ; KDRatm ; SRbc   ; BCatm  "));
    if (air_dist_module == "vr483")
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pRK   ; pZK   ; pKDR  ; pBCref; BPsr   ; MKzk km; MKzk pl; ZKrk gp; ZKrk dp; ZKkdr  ; ZKkdr d; MKkdr d; KDRbc  ; KDRatm ; KDRatmd; SRbc f ; SRbc s ; BCatm  ; poz d ; poz gp; poz up"));
}

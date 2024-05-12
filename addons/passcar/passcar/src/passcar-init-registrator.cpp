#include    "passcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::initRegistrator()
{
    registrator = new Registrator();
    registrator->setFileName(QString("passcar_%1").arg(idx));
    registrator->read_custom_config(config_dir + QDir::separator() + "registrator");
    registrator->init();

    if (air_dist_module == "vr292")
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pKDR  ; BPsr   ; BPkdr  ; KDRatm ; SRbc   ; BCatm  "));
    if (air_dist_module == "vr242")
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pUK   ; BPsr   ; BPuk   ; SRbc   ; BCatm  ; BPbc   ; BPatm  ; v11; v12; v1 ; v2 ; vb ; vs ; vw "));
}

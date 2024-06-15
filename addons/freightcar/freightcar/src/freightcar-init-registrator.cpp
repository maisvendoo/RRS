#include    "freightcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::initRegistrator(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    registrator = new Registrator();
    registrator->setFileName(QString("freightcar_%1").arg(idx));
    registrator->setReplaceDotByComma(true);
    registrator->read_config("registrator", custom_cfg_dir);
    registrator->init();

//    if (air_dist_module == "vr483")
//        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pRK   ; pZK   ; pKDR  ; pBCref; BPsr   ; MKzk km; MKzk pl; ZKrk gp; ZKrk pd; MKrk pd; ZKkdr  ; ZKkdr d; MKkdr d; KDRbc  ; KDRatm ; KDRatmd; SRbc f ; SRbc s ; BCatm  ; poz d ; poz gp; poz up"));
}

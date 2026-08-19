#include    "passcar.h"

#include    <QDir>

#include    "registrator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::initRegistrator(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    registrator = new Registrator();
    registrator->setFileName(QString("passcar_%1").arg(model_idx));
    registrator->setReplaceDotByComma(true);
    registrator->init();

    if (air_dist_module == "vr292")
    {
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pKDR  ; BPsr   ; BPkdr  ; KDRatm ; SRbc   ; BCatm  ; BPatm  ; mainS  ; disjZ  ; zdiff  ; zdA2 ; dopSR; vemrg"));
    }
    else if (air_dist_module == "vr242")
    {
        registrator->print(QString("  time  ; pBP   ; pBC   ; pSR   ; pUK   ; BPsr   ; BPuk   ; SRbc   ; BCatm  ; BPbc   ; BPatm  ; v11; v12; v1 ; v2 ; vb ; vs ; vw "));
    }
}

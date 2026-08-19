#include    "vl60pk.h"

#include    <QDir>

#include "coupling.h"
#include "coupling-operating-rod.h"

#include <core/load_module.h>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initCouplings(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) custom_cfg_dir;

    // Сцепные устройства
    coupling_fwd = LOAD_MODULE(Coupling,
        modules_dir + QDir::separator() + coupling_module_name);
    coupling_fwd->read_config(coupling_config_name);
    forward_connectors.push_back(coupling_fwd);

    coupling_bwd = LOAD_MODULE(Coupling,
        modules_dir + QDir::separator() + coupling_module_name);
    coupling_bwd->read_config(coupling_config_name);
    backward_connectors.push_back(coupling_bwd);

    // Расцепные рычаги
    oper_rod_fwd = new OperatingRod();
    oper_rod_fwd->read_config("coupling-operating-rod");
    oper_rod_bwd = new OperatingRod();
    oper_rod_bwd->read_config("coupling-operating-rod");
}

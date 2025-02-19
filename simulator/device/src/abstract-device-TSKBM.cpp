#include    "abstract-device-TSKBM.h"
#include    <QLibrary>

AbstractDeviceTSKBM::AbstractDeviceTSKBM(QObject *parent) : Device(parent)
{

}

AbstractDeviceTSKBM::~AbstractDeviceTSKBM()
{

}

void AbstractDeviceTSKBM::step(double t, double dt)
{
    Device::step(t, dt);
}

void AbstractDeviceTSKBM::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

AbstractDeviceTSKBM *loadPluginTSKBMDevice(QString lib_path)
{
    AbstractDeviceTSKBM *plugin_TSKBM_device = nullptr;

    QLibrary lib(lib_path);

    if(lib.load())
    {
        GetPluginTSKBMDevice getPluginTSKBMDevice = reinterpret_cast<GetPluginTSKBMDevice>(lib.resolve("getPluginTSKBMDevice"));

        if(getPluginTSKBMDevice)
        {
            plugin_TSKBM_device = getPluginTSKBMDevice();
        }
    }

    return plugin_TSKBM_device;
}

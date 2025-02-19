#include    "safety-device.h"
#include    <QLibrary>

SafetyDevice::SafetyDevice(QObject *parent) : Device(parent)
{

}

SafetyDevice::~SafetyDevice()
{

}

bool SafetyDevice::init()
{
    return true;
}

void SafetyDevice::step(double t, double dt)
{
    Device::step(t, dt);
}

void SafetyDevice::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

bool SafetyDevice::isDisplayON() const
{
    return true;
}

void SafetyDevice::setReversorDirection(int reversor_direction)
{
    (void) reversor_direction;
}

int SafetyDevice::getStationIndex() const
{
    return 0;
}

SafetyDevice *loadSafetyDevice(QString lib_path)
{
    SafetyDevice *safety_device = nullptr;

    QLibrary lib(lib_path);

    if(lib.load())
    {
        GetSafetyDevice getSafetyDevice = reinterpret_cast<GetSafetyDevice>(lib.resolve("getSafetyDevice"));

        if(getSafetyDevice)
        {
            safety_device = getSafetyDevice();
        }
    }

    return safety_device;
}

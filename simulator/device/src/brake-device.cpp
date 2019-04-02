#include    "brake-device.h"

BrakeDevice::BrakeDevice(QObject *parent) : Device (parent)
{

}

BrakeDevice::~BrakeDevice()
{

}

void BrakeDevice::init(double pTM)
{
    Q_UNUSED(pTM)
}

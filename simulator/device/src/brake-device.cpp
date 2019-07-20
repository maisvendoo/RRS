#include    "brake-device.h"

BrakeDevice::BrakeDevice(QObject *parent) : Device (parent)
{

}

BrakeDevice::~BrakeDevice()
{

}

void BrakeDevice::init(double pTM, double pFL)
{
    Q_UNUSED(pTM)
    Q_UNUSED(pFL)
}

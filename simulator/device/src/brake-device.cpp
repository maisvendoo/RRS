#include    "brake-device.h"

BrakeDevice::BrakeDevice(QObject *parent) : Device (parent)
{

}

BrakeDevice::~BrakeDevice()
{

}

void BrakeDevice::init(double pBP, double pFL)
{
    Q_UNUSED(pBP)
    Q_UNUSED(pFL)
}
